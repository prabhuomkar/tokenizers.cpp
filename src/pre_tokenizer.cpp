// Copyright 2024 Omkar Prabhu
#include "tokenizers/pre_tokenizer.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

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

bool is_whitespace(UChar32 c) { return u_isspace(c); }

bool is_bert_punc(UChar32 c) {
  return u_ispunct(c) || ispunct(static_cast<unsigned char>(c));
}

std::vector<int> find_matches(icu::UnicodeString input,
                              std::function<bool(UChar32)> match_fn) {
  std::vector<int> matches;
  for (int i = 0; i < input.length(); ++i) {
    if (match_fn(input.char32At(i))) {
      matches.push_back(i);
    }
  }
  return matches;
}

std::vector<Split> split(std::vector<Split> splits,
                         std::function<bool(UChar32)> split_fn,
                         SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::vector<Split> new_splits;
  for (Split split : splits) {
    int offset = 0;
    icu::UnicodeString split_unciode_str =
        icu::UnicodeString::fromUTF8(split.normalized);
    std::vector<int> matches = find_matches(split_unciode_str, split_fn);
    for (int match : matches) {
      if (split_unciode_str.tempSubStringBetween(offset, match).length() != 0) {
        std::string split_normalized;
        split_unciode_str.tempSubStringBetween(offset, match)
            .toUTF8String(split_normalized);
        new_splits.push_back(
            Split(split_normalized,
                  {offset + split.offsets.first, match + split.offsets.first}));
      }
      switch (pattern) {
        case REMOVED:
          break;
        case ISOLATED:
          std::string split_normalized;
          split_unciode_str.tempSubStringBetween(match, match + 1)
              .toUTF8String(split_normalized);
          new_splits.push_back(Split(
              split_normalized,
              {match + split.offsets.first, match + split.offsets.first + 1}));
          break;
      }
      offset = match + 1;
    }
    if (offset <= split_unciode_str.length() - 1) {
      std::string split_normalized;
      split_unciode_str.tempSubStringBetween(offset, split_unciode_str.length())
          .toUTF8String(split_normalized);
      new_splits.push_back(
          Split(split_normalized,
                {offset + split.offsets.first,
                 split_unciode_str.length() + split.offsets.first}));
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
