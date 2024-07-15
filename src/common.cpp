// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/common.h"

#include <string>
#include <utility>
#include <vector>

#include "simdjson.h"

Token::Token() {}

Token::Token(int id, std::string value, std::pair<int, int> offsets)
    : id(id), value(value), offsets(offsets) {}

std::string convert_to_string(std::wstring sequence) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(sequence);
}

std::wstring convert_from_string(std::string sequence) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(sequence);
}
