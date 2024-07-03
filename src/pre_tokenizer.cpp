// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/pre_tokenizer.h"

#include <iostream>
#include <string>
#include <unordered_map>

PRE_TOKENIZER get_pre_tokenizer(std::string_view type) {
  static const std::unordered_map<std::string_view, PRE_TOKENIZER> types = {
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

BertPreTokenizer::BertPreTokenizer() {
  std::cout << "Initialized PreTokenizer: BertPreTokenizer" << std::endl;
}
