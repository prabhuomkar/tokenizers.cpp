// Copyright 2024 Omkar Prabhu
#include "tokenizers/normalizer.h"

#include <unicode/normlzr.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

NORMALIZER get_normalizer(std::string type) {
  static const std::unordered_map<std::string, NORMALIZER> types = {
      {"Sequence", SEQUENCE_NORMALIZER},
      {"BertNormalizer", BERT_NORMALIZER},
      {"Lowercase", LOWERCASE_NORMALIZER},
      {"Prepend", PREPEND_NORMALIZER},
      {"NFC", NFC_NORMALIZER},
      {"NFD", NFD_NORMALIZER},
      {"NFKC", NFKC_NORMALIZER},
      {"NFKD", NFKD_NORMALIZER},
      {"Precompiled", PRECOMPILED_NORMALIZER},
      {"Replace", REPLACE_NORMALIZER},
      {"Strip", STRIP_NORMALIZER},
      {"StripAccents", STRIP_ACCENTS_NORMALIZER}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_NORMALIZER;
}

NormalizedString::NormalizedString(std::wstring normalized)
    : normalized(normalized) {
  icu::UnicodeString unicode_normalized = icu::UnicodeString::fromUTF32(
      reinterpret_cast<const UChar32*>(normalized.c_str()),
      normalized.length());
  int idx = 0;
  for (int i = 0; i < unicode_normalized.length(); i++) {
    icu::UnicodeString unicode_normalized_str;
    unicode_normalized_str.append(unicode_normalized.char32At(i));
    std::string normalized_str;
    unicode_normalized_str.toUTF8String(normalized_str);
    offset_ranges.push_back({idx, normalized_str.length()});
    idx += normalized_str.length();
    for (int j = 0; j < normalized_str.length(); j++) {
      offsets.push_back({i, i + 1});
    }
  }
}

NormalizedString::NormalizedString(std::wstring normalized,
                                   std::vector<std::pair<int, int>> offsets)
    : normalized(normalized), offsets(offsets) {}

std::wstring NormalizedString::get() { return normalized; }

std::unique_ptr<Normalizer> with_normalizer(
    simdjson::ondemand::object normalizer_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(normalizer_params["type"].get_string()));
  if (get_normalizer(type) == SEQUENCE_NORMALIZER) {
    simdjson::ondemand::array seq_normalizers_params =
        normalizer_params["normalizers"].get_array();
    std::vector<std::unique_ptr<Normalizer>> seq_normalizers;
    for (simdjson::ondemand::value val : seq_normalizers_params) {
      simdjson::ondemand::object seq_normalizer_params = val.get_object();
      std::unique_ptr<Normalizer> seq_normalizer =
          with_normalizer(seq_normalizer_params);
      if (seq_normalizer != nullptr) {
        seq_normalizers.push_back(std::move(seq_normalizer));
      }
    }
    return std::make_unique<SequenceNormalizer>(
        SequenceNormalizer(std::move(seq_normalizers)));
  } else if (get_normalizer(type) == NFD_NORMALIZER) {
    return std::make_unique<NFD>(NFD());
  } else if (get_normalizer(type) == PREPEND_NORMALIZER) {
    val = normalizer_params["prepend"].value();
    std::string prepend =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(val.get_string()));
    return std::make_unique<Prepend>(Prepend(prepend));
  } else if (get_normalizer(type) == REPLACE_NORMALIZER) {
    val = normalizer_params["pattern"].value();
    std::string pattern =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(
                              val["String"].value().get_string()));
    val = normalizer_params["content"].value();
    std::string content =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(val.get_string()));
    return std::make_unique<Replace>(Replace(pattern, content));
  } else if (get_normalizer(type) == BERT_NORMALIZER) {
    val = normalizer_params["clean_text"].value();
    bool clean_text = val.type() == simdjson::ondemand::json_type::null
                          ? false
                          : static_cast<bool>(val.get_bool());

    val = normalizer_params["handle_chinese_chars"].value();
    bool handle_chinese_chars =
        val.type() == simdjson::ondemand::json_type::null
            ? false
            : static_cast<bool>(val.get_bool());
    val = normalizer_params["strip_accents"].value();
    bool strip_accents = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());

    val = normalizer_params["lowercase"].value();
    bool lowercase = val.type() == simdjson::ondemand::json_type::null
                         ? false
                         : static_cast<bool>(val.get_bool());
    return std::make_unique<BertNormalizer>(BertNormalizer(
        clean_text, handle_chinese_chars, strip_accents, lowercase));
  }
  return nullptr;
}

void NormalizedString::transform(int i, std::string op, int n) {
  int start = offset_ranges[i].first;
  int limit = offset_ranges[i].second;
  if (op == "erase") {
    offsets.erase(offsets.begin() + start, offsets.begin() + start + limit);
    offset_ranges.erase(offset_ranges.begin() + i,
                        offset_ranges.begin() + i + 1);
  } else if (op == "add") {
    int start = offset_ranges[i].first;
    int limit = offset_ranges[i].second;
    std::pair<int, int> offset = offsets[start];
    std::vector<std::pair<int, int>> new_offsets(2, offset);
    offsets.insert(offsets.begin() + start, new_offsets.begin(),
                   new_offsets.end());
    std::vector<std::pair<int, int>> new_offset_ranges = {
        {start, 1}, {start + 1, limit}, {start + limit + 1, 1}};
    offset_ranges.erase(offset_ranges.begin() + i);
    offset_ranges.insert(offset_ranges.begin() + i, new_offset_ranges.begin(),
                         new_offset_ranges.end());
    for (int j = i + 3; j < offset_ranges.size(); j++) {
      offset_ranges[j].first =
          offset_ranges[j - 1].first + offset_ranges[j - 1].second;
    }
  } else if (op == "grow") {
    int start = offset_ranges[i].first;
    int limit = offset_ranges[i].second;
    std::pair<int, int> offset = offsets[start];
    std::vector<std::pair<int, int>> new_offsets(1, offset);
    offsets.insert(offsets.begin() + start, new_offsets.begin(),
                   new_offsets.end());
    std::vector<std::pair<int, int>> new_offset_ranges = {
        {start + limit + 1, 1}};
    offset_ranges.insert(offset_ranges.begin() + i, new_offset_ranges.begin(),
                         new_offset_ranges.end());
    for (int j = i + 1; j < offset_ranges.size(); j++) {
      offset_ranges[j].first =
          offset_ranges[j - 1].first + offset_ranges[j - 1].second;
    }
  } else if (op == "shrink") {
    int start = offset_ranges[i].first;
    int limit = offset_ranges[i].second;
    offsets.erase(offsets.begin() + start, offsets.begin() + start + limit - 1);
    offset_ranges.erase(offset_ranges.begin() + i);
    for (int j = i + 1; j < offset_ranges.size(); j++) {
      offset_ranges[j].first = offset_ranges[j].first - (limit - 1);
    }
  }
}

NFD::NFD() {}

NormalizedString NFD::normalize(NormalizedString normalized) const {
  // TODO(omkar): Handle errors
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString unicode_normalized = icu::UnicodeString::fromUTF32(
      reinterpret_cast<const UChar32*>(normalized.get().c_str()),
      normalized.get().length());
  icu::Normalizer::normalize(unicode_normalized, UNORM_NFD, 0,
                             unicode_normalized, status);
  std::wstring result;
  result.resize(unicode_normalized.countChar32());
  unicode_normalized.toUTF32(reinterpret_cast<UChar32*>(&result[0]),
                             unicode_normalized.countChar32(), status);
  int oi = 0, ni = 0;
  while (oi < normalized.get().length() && ni < result.length()) {
    if (normalized.normalized[oi] == result[ni]) {
      oi++;
      ni++;
    } else {
      normalized.transform(oi, "grow", 1);
      oi++;
      ni += 2;
    }
  }
  normalized.normalized = result;
  return normalized;
}

BertNormalizer::BertNormalizer(bool clean_text, bool handle_chinese_chars,
                               bool strip_accents, bool lowercase)
    : clean_text(clean_text),
      handle_chinese_chars(handle_chinese_chars),
      strip_accents(strip_accents),
      lowercase(lowercase) {}

NormalizedString BertNormalizer::normalize(NormalizedString normalized) const {
  if (clean_text) {
    normalized = do_clean_text(normalized);
  }
  if (handle_chinese_chars) {
    normalized = do_handle_chinese_chars(normalized);
  }
  if (strip_accents || lowercase) {
    normalized = do_strip_accents(normalized);
  }
  if (lowercase) {
    normalized = do_lowercase(normalized);
  }
  return normalized;
}

bool is_whitespace(wchar_t c) {
  switch (c) {
    case L'\t':
    case L'\n':
    case L'\r':
      return true;
    default:
      return std::iswspace(c);
  }
}

bool is_control(wchar_t c) {
  switch (c) {
    case L'\t':
    case L'\n':
    case L'\r':
      return false;
    default:
      return std::iswcntrl(c);
  }
}

bool is_chinese_char(wchar_t c) {
  return (c >= 0x4E00 && c <= 0x9FFF) || (c >= 0x3400 && c <= 0x4DBF) ||
         (c >= 0x20000 && c <= 0x2A6DF) || (c >= 0x2A700 && c <= 0x2B73F) ||
         (c >= 0x2B740 && c <= 0x2B81F) || (c >= 0x2B920 && c <= 0x2CEAF) ||
         (c >= 0xF900 && c <= 0xFAFF) || (c >= 0x2F800 && c <= 0x2FA1F);
}

NormalizedString BertNormalizer::do_clean_text(
    NormalizedString normalized) const {
  std::wstring result;
  int i = 0;
  for (wchar_t c : normalized.get()) {
    if (c != 0 && c != 0xFFFD && !is_control(c)) {
      if (is_whitespace(c)) {
        result.push_back(L' ');
      } else {
        result.push_back(c);
      }
    } else {
      normalized.transform(i, "erase", 0);
    }
    i++;
  }
  normalized.normalized = result;
  return normalized;
}

NormalizedString BertNormalizer::do_handle_chinese_chars(
    NormalizedString normalized) const {
  std::vector<std::pair<wchar_t, int>> new_chars;
  std::vector<int> transform_ids;
  int i = 0;
  for (wchar_t c : normalized.get()) {
    if (is_chinese_char(c)) {
      new_chars.insert(new_chars.end(), {{L' ', 0}, {c, 1}, {L' ', 1}});
      transform_ids.push_back(i);
    } else {
      new_chars.push_back({c, 0});
    }
    i++;
  }
  int multi = 0;
  for (auto ti : transform_ids) {
    ti += multi;
    multi += 2;
    normalized.transform(ti, "add", 2);
  }
  std::wstring result = normalized.get();
  i = 0;
  for (const auto& change : new_chars) {
    if (change.second > 0) {
      result.insert(i, 1, change.first);
    } else if (change.second < 0) {
      i += change.second;
      result.insert(i, 1, change.first);
    } else {
      result[i] = change.first;
    }
    i++;
  }
  normalized.normalized = result;
  return normalized;
}

NormalizedString BertNormalizer::do_strip_accents(
    NormalizedString normalized) const {
  auto nfd_normalized = NFD().normalize(normalized);
  std::wstring result;
  int i = 0;
  for (wchar_t c : nfd_normalized.normalized) {
    if (u_charType(static_cast<UChar32>(c)) != U_NON_SPACING_MARK) {
      result.push_back(c);
    } else {
      nfd_normalized.transform(i, "shrink", 0);
    }
    i++;
  }
  nfd_normalized.normalized = result;
  return nfd_normalized;
}

NormalizedString BertNormalizer::do_lowercase(
    NormalizedString normalized) const {
  std::wstring result = normalized.get();
  std::transform(result.begin(), result.end(), result.begin(), std::towlower);
  normalized.normalized = result;
  return normalized;
}

SequenceNormalizer::SequenceNormalizer(
    std::vector<std::unique_ptr<Normalizer>> normalizers)
    : normalizers(std::move(normalizers)) {}

NormalizedString SequenceNormalizer::normalize(
    NormalizedString normalized) const {
  for (const std::unique_ptr<Normalizer>& normalizer : normalizers) {
    normalized = normalizer->normalize(normalized);
  }
  return normalized;
}

Prepend::Prepend(std::string prepend) : prepend(prepend) {}

NormalizedString Prepend::normalize(NormalizedString normalized) const {
  int start = 0, end = 0;
  std::wstring result, current_word;
  while (end != std::wstring::npos) {
    end = normalized.get().find(L" ", start);
    if (end == std::wstring::npos) {
      current_word = normalized.get().substr(start);
    } else {
      current_word = normalized.get().substr(start, end - start);
    }
    result += (convert_from_string(prepend) + current_word + L" ");
    start = end + 1;
  }
  normalized.normalized = result;
  return normalized;
}

Replace::Replace(std::string pattern, std::string content)
    : pattern(pattern), content(content) {}

NormalizedString Replace::normalize(NormalizedString normalized) const {
  std::wregex regex_pattern(convert_from_string(pattern));
  std::wstring result = std::regex_replace(normalized.get(), regex_pattern,
                                           convert_from_string(content));
  normalized.normalized = result;
  return normalized;
}
