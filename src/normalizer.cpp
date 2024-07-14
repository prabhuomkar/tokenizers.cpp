// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/normalizer.h"

#include <unicode/normlzr.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "simdjson.h"

NORMALIZER get_normalizer(std::string type) {
  static const std::unordered_map<std::string, NORMALIZER> types = {
      {"BertNormalizer", BERT_NORMALIZER},
      {"Lowercase", LOWERCASE_NORMALIZER},
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

NormalizerConfig::NormalizerConfig(
    simdjson::ondemand::object normalizer_params) {
  simdjson::ondemand::value val;
  type = std::string(
      static_cast<std::string_view>(normalizer_params["type"].get_string()));

  val = normalizer_params["clean_text"].value();
  clean_text = val.type() == simdjson::ondemand::json_type::null
                   ? false
                   : static_cast<bool>(val.get_bool());

  val = normalizer_params["handle_chinese_chars"].value();
  handle_chinese_chars = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());
  val = normalizer_params["strip_accents"].value();
  strip_accents = val.type() == simdjson::ondemand::json_type::null
                      ? false
                      : static_cast<bool>(val.get_bool());

  val = normalizer_params["lowercase"].value();
  lowercase = val.type() == simdjson::ondemand::json_type::null
                  ? false
                  : static_cast<bool>(val.get_bool());
}

Normalizer::Normalizer() {}

std::wstring Normalizer::normalize(std::wstring normalized) { return L""; }

NFD::NFD() {}

std::wstring NFD::normalize(std::wstring normalized) {
  // TODO(omkar): Handle errors
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString unicode_normalized = icu::UnicodeString::fromUTF32(
      reinterpret_cast<const UChar32*>(normalized.c_str()),
      normalized.length());
  icu::Normalizer::normalize(unicode_normalized, UNORM_NFD, 0,
                             unicode_normalized, status);
  std::wstring result;
  unicode_normalized.toUTF32(reinterpret_cast<UChar32*>(&result[0]),
                             unicode_normalized.countChar32(), status);
  return result;
}

BertNormalizer::BertNormalizer(bool clean_text, bool handle_chinese_chars,
                               bool strip_accents, bool lowercase)
    : clean_text(clean_text),
      handle_chinese_chars(handle_chinese_chars),
      strip_accents(strip_accents),
      lowercase(lowercase) {
  std::cout << "Initialized Normalizer: BertNormalizer" << std::endl;
  std::cout << "params:" << clean_text << " " << handle_chinese_chars << " "
            << strip_accents << " " << lowercase << std::endl;
}

std::wstring BertNormalizer::normalize(std::wstring normalized) {
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

std::wstring BertNormalizer::do_clean_text(std::wstring normalized) {
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

std::wstring BertNormalizer::do_handle_chinese_chars(std::wstring normalized) {
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

std::wstring BertNormalizer::do_strip_accents(std::wstring normalized) {
  normalized = NFD().normalize(normalized);
  std::wstring result;
  for (wchar_t c : normalized) {
    if (u_charType(static_cast<UChar32>(c)) != U_NON_SPACING_MARK) {
      result.push_back(c);
    }
  }
  return result;
}

std::wstring BertNormalizer::do_lowercase(std::wstring normalized) {
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 std::towlower);
  return normalized;
}
