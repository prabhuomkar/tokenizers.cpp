// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/normalizer.h"

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
#include "tokenizers.cpp/common.h"

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

std::wstring Normalizer::normalize(std::wstring normalized) const {
  return L"";
}

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

NFD::NFD() {}

std::wstring NFD::normalize(std::wstring normalized) const {
  // TODO(omkar): Handle errors
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString unicode_normalized = icu::UnicodeString::fromUTF32(
      reinterpret_cast<const UChar32*>(normalized.c_str()),
      normalized.length());
  icu::Normalizer::normalize(unicode_normalized, UNORM_NFD, 0,
                             unicode_normalized, status);
  std::wstring result;
  result.resize(unicode_normalized.countChar32());
  unicode_normalized.toUTF32(reinterpret_cast<UChar32*>(&result[0]),
                             unicode_normalized.countChar32(), status);
  return result;
}

BertNormalizer::BertNormalizer(bool clean_text, bool handle_chinese_chars,
                               bool strip_accents, bool lowercase)
    : clean_text(clean_text),
      handle_chinese_chars(handle_chinese_chars),
      strip_accents(strip_accents),
      lowercase(lowercase) {}

std::wstring BertNormalizer::normalize(std::wstring normalized) const {
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

std::wstring BertNormalizer::do_clean_text(std::wstring normalized) const {
  std::cout << "cleaning text" << std::endl;
  std::wstring result;
  for (wchar_t c : normalized) {
    if (c != 0 && c != 0xFFFD && !is_control(c)) {
      if (is_whitespace(c)) {
        result.push_back(L' ');
      } else {
        result.push_back(c);
      }
    }
  }
  std::transform(result.begin(), result.end(), result.begin(),
                 [](wchar_t c) { return is_whitespace(c) ? ' ' : c; });
  return result;
}

std::wstring BertNormalizer::do_handle_chinese_chars(
    std::wstring normalized) const {
  std::vector<std::pair<wchar_t, int>> new_chars;
  for (wchar_t c : normalized) {
    if (is_chinese_char(c)) {
      new_chars.insert(new_chars.end(), {{L' ', 0}, {c, 1}, {L' ', 1}});
    } else {
      new_chars.push_back({c, 0});
    }
  }
  // TODO(omkar): calculate offsets
  std::wstring result;
  for (auto ci : new_chars) {
    result.push_back(ci.first);
  }
  return result;
}

std::wstring BertNormalizer::do_strip_accents(std::wstring normalized) const {
  std::wstring nfd_normalized = NFD().normalize(normalized);
  std::wstring result;
  for (wchar_t c : nfd_normalized) {
    if (u_charType(static_cast<UChar32>(c)) != U_NON_SPACING_MARK) {
      result.push_back(c);
    }
  }
  return result;
}

std::wstring BertNormalizer::do_lowercase(std::wstring normalized) const {
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 std::towlower);
  return normalized;
}

SequenceNormalizer::SequenceNormalizer(
    std::vector<std::unique_ptr<Normalizer>> normalizers)
    : normalizers(std::move(normalizers)) {}

std::wstring SequenceNormalizer::normalize(std::wstring normalized) const {
  for (const std::unique_ptr<Normalizer>& normalizer : normalizers) {
    normalized = normalizer->normalize(normalized);
  }
  return normalized;
}

Prepend::Prepend(std::string prepend) : prepend(prepend) {}

std::wstring Prepend::normalize(std::wstring normalized) const {
  int start = 0, end = 0;
  std::wstring result, current_word;
  while (end != std::wstring::npos) {
    end = normalized.find(L" ", start);
    if (end == std::wstring::npos) {
      current_word = normalized.substr(start);
    } else {
      current_word = normalized.substr(start, end - start);
    }
    result += (convert_from_string(prepend) + current_word + L" ");
    start = end + 1;
  }
  return result;
}

Replace::Replace(std::string pattern, std::string content)
    : pattern(pattern), content(content) {}

std::wstring Replace::normalize(std::wstring normalized) const {
  std::wregex regex_pattern(convert_from_string(pattern));
  return std::regex_replace(normalized, regex_pattern,
                            convert_from_string(content));
}
