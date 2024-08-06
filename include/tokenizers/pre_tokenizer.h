// Copyright 2024 Omkar Prabhu
#pragma once

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
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
  SEQUENCE_PRE_TOKENIZER,
  UNKNOWN_PRE_TOKENIZER
};

PRE_TOKENIZER
get_pre_tokenizer(std::string type);

class PreTokenizedString {
 public:
  NormalizedString normalized;
  std::vector<Split> splits;
  explicit PreTokenizedString(const NormalizedString& normalized);
  void split(std::function<std::vector<std::pair<std::pair<int, int>, bool>>(
                 icu::UnicodeString)>
                 split_fn,
             SPLIT_DELIMITER_BEHAVIOR pattern);
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

class SequencePreTokenizer : public PreTokenizer {
 public:
  explicit SequencePreTokenizer(
      std::vector<std::unique_ptr<PreTokenizer>> pretokenizers);
  PreTokenizedString pre_tokenize(
      PreTokenizedString pre_tokenized) const override;

 private:
  std::vector<std::unique_ptr<PreTokenizer>> pretokenizers;
};

class SplitPreTokenizer : public PreTokenizer {
 public:
  explicit SplitPreTokenizer(const std::string& pattern,
                             const std::string& behavior, bool invert);
  PreTokenizedString pre_tokenize(
      PreTokenizedString pre_tokenized) const override;

 private:
  std::string pattern;
  SPLIT_DELIMITER_BEHAVIOR behavior;
  bool invert;
};

class ByteLevelPreTokenizer : public PreTokenizer {
 public:
  explicit ByteLevelPreTokenizer(bool add_prefix_space, bool use_regex);
  PreTokenizedString pre_tokenize(
      PreTokenizedString pre_tokenized) const override;

 private:
  bool add_prefix_space;
  bool use_regex;
  std::string regex;
  std::unordered_map<uint16_t, std::string> BYTES_CHAR;
};
