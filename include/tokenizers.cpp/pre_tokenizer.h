// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

#include "simdjson.h"

enum PRE_TOKENIZER {
  BERT_PRE_TOKENIZER,
  BYTE_LEVEL_PRE_TOKENIZER,
  CHAR_DELIMITER_SPLIT_PRE_TOKENIZER,
  DIGITS_PRE_TOKENIZER,
  METASPACE_PRE_TOKENIZER,
  PUNCTUATION_PRE_TOKENIZER,
  SPLIT_PRE_TOKENIZER,
  UNICODE_SCRIPTS_PRE_TOKENIZER,
  WHITESPACE_PRE_TOKENIZER,
  WHITESPACE_SPLIT_PRE_TOKENIZER,
  UNKNOWN_PRE_TOKENIZER
};

enum SPLIT_DELIMITER_BEHAVIOR { REMOVED, ISOLATED };

PRE_TOKENIZER
get_pre_tokenizer(std::string type);

class PreTokenizer {
 public:
  PreTokenizer();
  std::string pre_tokenize(std::wstring normalized);
};

PreTokenizer with_pre_tokenizer(
    simdjson::ondemand::object pre_tokenizer_params);

class BertPreTokenizer : public PreTokenizer {
 public:
  BertPreTokenizer();
  std::string pre_tokenize(std::wstring normalized);
};
