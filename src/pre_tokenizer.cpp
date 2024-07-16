// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/pre_tokenizer.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "simdjson.h"
#include "tokenizers.cpp/common.h"

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

std::unique_ptr<PreTokenizer> with_pre_tokenizer(
    simdjson::ondemand::object pre_tokenizer_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(pre_tokenizer_params["type"].get_string()));
  if (get_pre_tokenizer(type) == BERT_PRE_TOKENIZER) {
    return std::make_unique<BertPreTokenizer>(BertPreTokenizer());
  }
  return nullptr;
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

std::string BertPreTokenizer::pre_tokenize(std::wstring normalized) const {
  std::string pre_tokenized = convert_to_string(normalized);
  pre_tokenized =
      split(pre_tokenized, is_whitespace, SPLIT_DELIMITER_BEHAVIOR::REMOVED);
  pre_tokenized =
      split(pre_tokenized, is_whitespace, SPLIT_DELIMITER_BEHAVIOR::ISOLATED);
  return pre_tokenized;
}
