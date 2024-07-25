// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"
#include "tokenizers/normalizer.h"

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

class PreTokenizedString {
 public:
  NormalizedString normalized;
  std::vector<Split> splits;
  explicit PreTokenizedString(NormalizedString normalized);
};

class PreTokenizer {
 public:
  virtual ~PreTokenizer() = default;
  virtual PreTokenizedString pre_tokenize(
      PreTokenizedString pre_tokenized) const = 0;
};

std::unique_ptr<PreTokenizer> with_pre_tokenizer(
    simdjson::ondemand::object pre_tokenizer_params);

class BertPreTokenizer : public PreTokenizer {
 public:
  BertPreTokenizer();
  PreTokenizedString pre_tokenize(
      PreTokenizedString pre_tokenized) const override;
};
