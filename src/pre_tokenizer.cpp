// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/pre_tokenizer.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

std::vector<int> find_matches(std::string input,
                              std::function<bool(char)> match_fn) {
  std::vector<int> matches;
  for (int i = 0; i < input.size(); i++) {
    if (match_fn(input[i])) {
      matches.push_back(i);
    }
  }
  return matches;
}

std::vector<Split> split(std::vector<Split> splits,
                         std::function<bool(char)> split_fn,
                         SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::vector<Split> new_splits;
  for (Split split : splits) {
    int offset = 0;
    std::vector<int> matches = find_matches(split.normalized, split_fn);
    for (int match : matches) {
      if (split.normalized.substr(offset, match - offset).size() != 0) {
        new_splits.push_back(
            Split(split.normalized.substr(offset, match - offset),
                  {offset + split.offsets.first, match + split.offsets.first}));
      }
      switch (pattern) {
        case REMOVED:
          break;
        case ISOLATED:
          new_splits.push_back(Split(
              split.normalized.substr(match, 1),
              {match + split.offsets.first, match + split.offsets.first + 1}));
          break;
      }
      offset = match + 1;
    }
    if (offset <= split.normalized.length() - 1) {
      new_splits.push_back(Split(
          split.normalized.substr(offset, split.normalized.length() - offset),
          {offset + split.offsets.first,
           split.normalized.length() + split.offsets.first}));
    }
  }
  return new_splits;
}

BertPreTokenizer::BertPreTokenizer() {}

std::vector<Split> BertPreTokenizer::pre_tokenize(
    std::wstring normalized) const {
  std::string pre_tokenized = convert_to_string(normalized);
  std::vector<Split> splits = {
      Split(pre_tokenized, {0, pre_tokenized.length()})};
  splits = split(splits, is_whitespace, SPLIT_DELIMITER_BEHAVIOR::REMOVED);
  splits = split(splits, is_bert_punc, SPLIT_DELIMITER_BEHAVIOR::ISOLATED);
  return splits;
}
