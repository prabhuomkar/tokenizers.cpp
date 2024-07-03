// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

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

PRE_TOKENIZER get_pre_tokenizer(std::string_view type);

class PreTokenizer {
 public:
  PreTokenizer();
};

class BertPreTokenizer : public PreTokenizer {
 public:
  BertPreTokenizer();
};
