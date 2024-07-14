// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/pre_tokenizer.h"

#include <codecvt>
#include <functional>
#include <iostream>
#include <locale>
#include <string>
#include <unordered_map>

PRE_TOKENIZER get_pre_tokenizer(std::string type) {
  static const std::unordered_map<std::string, PRE_TOKENIZER> types = {
      {"BertPreTokenizer", BERT_PRE_TOKENIZER},
      {"ByteLevel", BYTE_LEVEL_PRE_TOKENIZER},
      {"CharDelimiterSplit", CHAR_DELIMITER_SPLIT_PRE_TOKENIZER},
      {"Digits", DIGITS_PRE_TOKENIZER},
      {"Metaspace", METASPACE_PRE_TOKENIZER},
      {"Punctuation", PUNCTUATION_PRE_TOKENIZER},
      {"Split", SPLIT_PRE_TOKENIZER},
      {"UnicodeScripts", UNICODE_SCRIPTS_PRE_TOKENIZER},
      {"Whitespace", WHITESPACE_PRE_TOKENIZER},
      {"WhitespaceSplit", WHITESPACE_SPLIT_PRE_TOKENIZER}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_PRE_TOKENIZER;
}

PreTokenizer::PreTokenizer() {}

std::string PreTokenizer::pre_tokenize(std::wstring normalized) { return ""; }

std::string convert_to_string(std::wstring sequence) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(sequence);
}

bool is_whitespace(char c) { return isspace(static_cast<unsigned char>(c)); }

bool is_bert_punc(char c) { return ispunct(static_cast<unsigned char>(c)); }

std::string split(std::string input, std::function<bool(char)> split_fn,
                  SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::string result;
  for (char c : input) {
    if (split_fn(c)) {
      switch (pattern) {
        case REMOVED:
          result += "";
          break;
        case ISOLATED:
          result += c;
          break;
      }
    } else {
      result += c;
    }
  }
  return result;
}

BertPreTokenizer::BertPreTokenizer() {}

std::string BertPreTokenizer::pre_tokenize(std::wstring normalized) {
  std::string pre_tokenized = convert_to_string(normalized);
  pre_tokenized =
      split(pre_tokenized, is_whitespace, SPLIT_DELIMITER_BEHAVIOR::REMOVED);
  pre_tokenized =
      split(pre_tokenized, is_whitespace, SPLIT_DELIMITER_BEHAVIOR::ISOLATED);
  return pre_tokenized;
}
