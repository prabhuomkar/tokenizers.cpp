// Copyright 2024 Omkar Prabhu
#include "tokenizers/common.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "simdjson.h"

Token::Token() {}

Token::Token(int id, std::string value, std::pair<int, int> offsets)
    : id(id), value(value), offsets(offsets) {}

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

Encoding::Encoding(std::vector<int> ids, std::vector<int> type_ids,
                   std::vector<std::string> tokens,
                   std::vector<std::optional<int>> words,
                   std::vector<std::pair<int, int>> offsets,
                   std::vector<int> special_tokens_mask,
                   std::vector<int> attention_mask)
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
