// Copyright 2024 Omkar Prabhu
#include "tokenizers/normalizer.h"

#include <unicode/normlzr.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <functional>
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

void debug(NormalizedString normalized) {
  std::cout << normalized.offset_ranges.size() << std::endl;
  for (auto ofr : normalized.offset_ranges) {
    std::cout << "(" << ofr.first << "," << ofr.second << "), ";
  }
  std::cout << std::endl;
  std::cout << normalized.offsets.size() << std::endl;
  for (auto ofr : normalized.offsets) {
    std::cout << "(" << ofr.first << "," << ofr.second << "), ";
  }
  std::cout << std::endl;
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

void NormalizedString::transform_range(std::pair<int, int> original_offsets,
                                       NormalizedString sub_normalized) {
  int original_start = original_offsets.first;
  int original_end = original_offsets.second;
  int offsets_start = offset_ranges[original_start].first;
  int offsets_end = offset_ranges[original_end].first;
  std::vector<std::pair<int, int>> new_offset_ranges;
  for (int i = 0; i < sub_normalized.offset_ranges.size(); i++) {
    if (i == 0) {
      sub_normalized.offset_ranges[i].first =
          offset_ranges[original_start].first;
    } else {
      sub_normalized.offset_ranges[i].first =
          sub_normalized.offset_ranges[i - 1].first +
          sub_normalized.offset_ranges[i - 1].second;
    }
    new_offset_ranges.push_back(sub_normalized.offset_ranges[i]);
  }
  int j = new_offset_ranges.size();
  for (int i = original_end; i < offset_ranges.size(); i++) {
    new_offset_ranges.push_back(
        {new_offset_ranges[j - 1].first + new_offset_ranges[j - 1].second,
         offset_ranges[i].second});
    j++;
  }
  std::pair<int, int> prev;
  for (int i = 0; i < sub_normalized.offsets.size(); i++) {
    if (i == 0) {
      prev = sub_normalized.offsets[i];
      sub_normalized.offsets[i].first = offsets[offsets_start].first;
      sub_normalized.offsets[i].second = offsets[offsets_start].second;
    } else {
      if (prev == sub_normalized.offsets[i]) {
        sub_normalized.offsets[i] = sub_normalized.offsets[i - 1];
      } else {
        prev = sub_normalized.offsets[i];
        sub_normalized.offsets[i] = {sub_normalized.offsets[i - 1].second,
                                     sub_normalized.offsets[i - 1].second + 1};
      }
    }
  }
  offset_ranges.erase(offset_ranges.begin() + original_start,
                      offset_ranges.end());
  offset_ranges.insert(offset_ranges.begin() + original_start,
                       new_offset_ranges.begin(), new_offset_ranges.end());
  offsets.erase(offsets.begin() + offsets_start, offsets.begin() + offsets_end);
  offsets.insert(offsets.begin() + offsets_start,
                 sub_normalized.offsets.begin(), sub_normalized.offsets.end());
}

void NormalizedString::transform(int i, std::string op, int n) {
  int start = offset_ranges[i].first;
  int limit = offset_ranges[i].second;
  if (op == "erase") {
    offsets.erase(offsets.begin() + start, offsets.begin() + start + limit);
    offset_ranges.erase(offset_ranges.begin() + i,
                        offset_ranges.begin() + i + 1);
  } else if (op == "pad") {
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
    std::vector<std::pair<int, int>> new_offset_ranges = {{start, 1},
                                                          {start + 1, 1}};
    offset_ranges.erase(offset_ranges.begin() + i);
    offset_ranges.insert(offset_ranges.begin() + i, new_offset_ranges.begin(),
                         new_offset_ranges.end());
  } else if (op == "shrink") {
    int start = offset_ranges[i].first;
    int limit = offset_ranges[i].second;
    offset_ranges.erase(offset_ranges.begin() + i);
  } else if (op == "add" || op == "replace") {
    int start = offset_ranges[i].first;
    int limit = offset_ranges[i].second;
    std::vector<std::pair<int, int>> new_offset_ranges = {{start, n}};
    if (op == "add") {
      new_offset_ranges.push_back({start + n, limit});
    }
    offset_ranges.erase(offset_ranges.begin() + i);
    offset_ranges.insert(offset_ranges.begin() + i, new_offset_ranges.begin(),
                         new_offset_ranges.end());
    for (int j = i + (op == "add" ? 2 : 1); j < offset_ranges.size(); j++) {
      offset_ranges[j].first =
          offset_ranges[j - 1].first + offset_ranges[j - 1].second;
    }
    std::vector<std::pair<int, int>> new_offsets((op == "add" ? n : n - 1),
                                                 {offsets[start]});
    offsets.insert(offsets.begin() + start, new_offsets.begin(),
                   new_offsets.end());
  }
}

NFD::NFD() {}

NormalizedString NFD::normalize(NormalizedString normalized) const {
  // TODO(omkar): Handle errors
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString unicode_normalized = icu::UnicodeString::fromUTF32(
      reinterpret_cast<const UChar32*>(normalized.normalized.c_str()),
      normalized.normalized.length());
  icu::Normalizer::normalize(unicode_normalized, UNORM_NFD, 0,
                             unicode_normalized, status);
  std::wstring result;
  result.resize(unicode_normalized.countChar32());
  unicode_normalized.toUTF32(reinterpret_cast<UChar32*>(&result[0]),
                             unicode_normalized.countChar32(), status);
  int oi = 0, ni = 0;
  std::vector<int> grow_ids;
  while (oi < normalized.normalized.length() && ni < result.length()) {
    if (normalized.normalized[oi] == result[ni]) {
      oi++;
      ni++;
    } else {
      grow_ids.push_back(oi);
      oi++;
      ni += 2;
    }
  }
  int multi = 0;
  for (auto i : grow_ids) {
    normalized.transform(i + multi, "grow", 0);
    multi += 1;
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
  for (wchar_t c : normalized.normalized) {
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
  for (wchar_t c : normalized.normalized) {
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
    normalized.transform(ti, "pad", 0);
  }
  std::wstring result = normalized.normalized;
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
  std::vector<int> shrink_ids;
  for (wchar_t c : nfd_normalized.normalized) {
    if (u_charType(static_cast<UChar32>(c)) != U_NON_SPACING_MARK) {
      result.push_back(c);
    } else {
      shrink_ids.push_back(i);
    }
    i++;
  }
  int multi = 0;
  for (auto i : shrink_ids) {
    nfd_normalized.transform(i + multi, "shrink", 0);
    multi -= 1;
  }
  nfd_normalized.normalized = result;
  return nfd_normalized;
}

NormalizedString BertNormalizer::do_lowercase(
    NormalizedString normalized) const {
  std::wstring result = normalized.normalized;
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
  std::wstring result = convert_from_string(prepend) + normalized.normalized;
  normalized.transform(0, "add", prepend.length());
  normalized.normalized = result;
  return normalized;
}

Replace::Replace(std::string pattern, std::string content)
    : pattern(pattern), content(content) {}

NormalizedString Replace::normalize(NormalizedString normalized) const {
  std::wregex regex_pattern(convert_from_string(pattern));
  std::vector<int> replace_ids;
  std::wsregex_iterator it(normalized.normalized.begin(),
                           normalized.normalized.end(), regex_pattern);
  std::wsregex_iterator end;
  std::wstring replace_content = convert_from_string(content);
  while (it != end) {
    std::wsmatch match = *it;
    replace_ids.push_back(match.position());
    normalized.normalized.erase(normalized.normalized.begin() +
                                match.position());
    normalized.normalized.insert(
        normalized.normalized.begin() + match.position(),
        replace_content.begin(), replace_content.end());
    ++it;
  }
  int multi = 0;
  for (auto i : replace_ids) {
    normalized.transform(i + multi, "replace", content.length());
    multi += replace_content.length();
  }
  return normalized;
}
