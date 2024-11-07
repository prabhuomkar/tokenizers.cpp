// Copyright 2024 Omkar Prabhu
#include "tokenizers/common.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <vector>

#include "simdjson.h"

Token::Token() : id(0), value(""), offsets({}) {}

Token::Token(int id, std::string value, std::pair<int, int> offsets)
    : id(id), value(value), offsets(offsets) {}

SPLIT_DELIMITER_BEHAVIOR get_split_delimiter_behavior(
    const std::string& behavior) {
  if (behavior == "Isolated") {
    return ISOLATED;
  }
  return REMOVED;
}

Split::Split() {}

Split::Split(std::string normalized, std::pair<int, int> offsets)
    : normalized(normalized), offsets(offsets) {}

Encoding::Encoding()
    : ids({}),
      type_ids({}),
      tokens({}),
      words({}),
      offsets({}),
      special_tokens_mask({}),
      attention_mask({}) {}

Encoding::Encoding(const std::vector<int>& ids,
                   const std::vector<int>& type_ids,
                   const std::vector<std::string>& tokens,
                   const std::vector<std::optional<int>>& words,
                   const std::vector<std::pair<int, int>>& offsets,
                   const std::vector<int>& special_tokens_mask,
                   const std::vector<int>& attention_mask)
    : ids(ids),
      type_ids(type_ids),
      tokens(tokens),
      words(words),
      offsets(offsets),
      special_tokens_mask(special_tokens_mask),
      attention_mask(attention_mask) {}

std::string convert_to_string(std::wstring sequence) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(sequence);
}

std::wstring convert_from_string(std::string sequence) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(sequence);
}

std::unordered_map<uint16_t, std::string> bytes_char() {
  std::unordered_map<uint16_t, std::string> result;
  std::vector<uint16_t> bs;
  for (uint16_t i = '!'; i <= '~'; ++i) {
    bs.push_back(i);
  }
  for (uint16_t i = 0xA1; i <= 0xAC; ++i) {
    bs.push_back(i);
  }
  for (uint16_t i = 0xAE; i <= 0xFF; ++i) {
    bs.push_back(i);
  }
  std::vector<uint32_t> cs;
  std::transform(bs.begin(), bs.end(), std::back_inserter(cs),
                 [](uint16_t b) { return static_cast<uint32_t>(b); });
  uint32_t n = 0;
  for (uint16_t b = 0; b <= 255; ++b) {
    if (std::find(bs.begin(), bs.end(), b) == bs.end()) {
      bs.push_back(b);
      cs.push_back((1 << 8) + n);
      ++n;
    }
  }
  for (size_t i = 0; i < bs.size(); ++i) {
    uint16_t byte = bs[i];
    uint32_t code_point = cs[i];
    if (code_point <= 0x10FFFF) {
      std::string str;
      icu::UnicodeString unicode_str;
      if (code_point <= 0xFFFF) {
        unicode_str.append(static_cast<UChar>(code_point));
      } else {
        code_point -= 0x10000;
        unicode_str.append(static_cast<UChar>((code_point >> 10) + 0xD800));
        unicode_str.append(static_cast<UChar>((code_point & 0x3FF) + 0xDC00));
      }
      unicode_str.toUTF8String(str);
      result[byte] = str;
    }
  }
  return result;
}
