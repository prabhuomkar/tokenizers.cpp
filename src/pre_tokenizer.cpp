// Copyright 2024 Omkar Prabhu
#include "tokenizers/pre_tokenizer.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"
#include "tokenizers/normalizer.h"

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
      {"Sequence", SEQUENCE_PRE_TOKENIZER},
      {"Whitespace", WHITESPACE_PRE_TOKENIZER},
      {"WhitespaceSplit", WHITESPACE_SPLIT_PRE_TOKENIZER}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_PRE_TOKENIZER;
}

PreTokenizedString::PreTokenizedString(const NormalizedString& normalized)
    : normalized(normalized),
      splits({Split(convert_to_string(normalized.normalized),
                    {0, normalized.normalized.length()})}) {}

std::unique_ptr<PreTokenizer> with_pre_tokenizer(
    simdjson::ondemand::object pre_tokenizer_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(pre_tokenizer_params["type"].get_string()));
  if (get_pre_tokenizer(type) == BERT_PRE_TOKENIZER) {
    return std::make_unique<BertPreTokenizer>(BertPreTokenizer());
  } else if (get_pre_tokenizer(type) == SEQUENCE_PRE_TOKENIZER) {
    simdjson::ondemand::array seq_pretokenizers_list =
        pre_tokenizer_params["pretokenizers"].get_array();
    std::vector<std::unique_ptr<PreTokenizer>> seq_pre_tokenizers;
    for (simdjson::ondemand::value pre_tokenizer_val : seq_pretokenizers_list) {
      simdjson::ondemand::object seq_pre_tokenizer_params =
          pre_tokenizer_val.get_object();
      std::unique_ptr<PreTokenizer> seq_pre_tokenizer =
          with_pre_tokenizer(seq_pre_tokenizer_params);
      if (seq_pre_tokenizer != nullptr) {
        seq_pre_tokenizers.push_back(std::move(seq_pre_tokenizer));
      }
    }
    return std::make_unique<SequencePreTokenizer>(
        SequencePreTokenizer(std::move(seq_pre_tokenizers)));
  } else if (get_pre_tokenizer(type) == SPLIT_PRE_TOKENIZER) {
    val = pre_tokenizer_params["pattern"].value();
    std::string pattern = std::string(
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : static_cast<std::string_view>(val["Regex"].value().get_string()));
    val = pre_tokenizer_params["behavior"].value();
    std::string behavior =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = pre_tokenizer_params["invert"].value();
    bool invert = val.type() == simdjson::ondemand::json_type::null
                      ? false
                      : static_cast<bool>(val.get_bool());
    return std::make_unique<SplitPreTokenizer>(
        SplitPreTokenizer(pattern, behavior, invert));
  } else if (get_pre_tokenizer(type) == BYTE_LEVEL_PRE_TOKENIZER) {
    val = pre_tokenizer_params["add_prefix_space"].value();
    bool add_prefix_space = val.type() == simdjson::ondemand::json_type::null
                                ? false
                                : static_cast<bool>(val.get_bool());
    val = pre_tokenizer_params["trim_offsets"].value();
    bool trim_offsets = val.type() == simdjson::ondemand::json_type::null
                            ? false
                            : static_cast<bool>(val.get_bool());
    val = pre_tokenizer_params["use_regex"].value();
    bool use_regex = val.type() == simdjson::ondemand::json_type::null
                         ? false
                         : static_cast<bool>(val.get_bool());
    return std::make_unique<ByteLevelPreTokenizer>(
        ByteLevelPreTokenizer(add_prefix_space, trim_offsets, use_regex));
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
std::vector<Split> split_normalized(Split original_split,
                                    std::function<bool(UChar32)> split_fn,
                                    SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::vector<Split> new_splits;
  int offset = 0;
  icu::UnicodeString unicode_normalized =
      icu::UnicodeString::fromUTF8(original_split.normalized);
  std::vector<int> matches = find_matches(unicode_normalized, split_fn);
  for (int match : matches) {
    if (unicode_normalized.tempSubStringBetween(offset, match).length() != 0) {
      std::string split_unicode_normalized;
      unicode_normalized.tempSubStringBetween(offset, match)
          .toUTF8String(split_unicode_normalized);
      new_splits.push_back(Split(split_unicode_normalized,
                                 {offset + original_split.offsets.first,
                                  match + original_split.offsets.first}));
    }
    switch (pattern) {
      case REMOVED:
        break;
      case ISOLATED:
        std::string split_unicode_normalized;
        unicode_normalized.tempSubStringBetween(match, match + 1)
            .toUTF8String(split_unicode_normalized);
        new_splits.push_back(
            Split(split_unicode_normalized,
                  {match + original_split.offsets.first,
                   match + original_split.offsets.first + +1}));
        break;
    }
    offset = match + 1;
  }
  if (offset <= unicode_normalized.length() - 1) {
    std::string split_unicode_normalized;
    unicode_normalized.tempSubStringBetween(offset, unicode_normalized.length())
        .toUTF8String(split_unicode_normalized);
    new_splits.push_back(
        Split(split_unicode_normalized,
              {offset + original_split.offsets.first,
               unicode_normalized.length() + original_split.offsets.first}));
  }
  return new_splits;
}

void PreTokenizedString::split(std::function<bool(UChar32)> split_fn,
                               SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::vector<Split> new_splits;
  for (auto org_split : splits) {
    if (org_split.tokens.size() != 0) {
      new_splits.push_back(org_split);
      continue;
    }

    std::vector<Split> new_normalized_splits =
        split_normalized(org_split, split_fn, pattern);
    new_splits.insert(new_splits.end(), new_normalized_splits.begin(),
                      new_normalized_splits.end());
  }
  splits = new_splits;
}

BertPreTokenizer::BertPreTokenizer() {}

PreTokenizedString BertPreTokenizer::pre_tokenize(
    PreTokenizedString pre_tokenized) const {
  pre_tokenized.split(is_whitespace, SPLIT_DELIMITER_BEHAVIOR::REMOVED);
  pre_tokenized.split(is_bert_punc, SPLIT_DELIMITER_BEHAVIOR::ISOLATED);
  return pre_tokenized;
}

SequencePreTokenizer::SequencePreTokenizer(
    std::vector<std::unique_ptr<PreTokenizer>> pretokenizers)
    : pretokenizers(std::move(pretokenizers)) {}

PreTokenizedString SequencePreTokenizer::pre_tokenize(
    PreTokenizedString pre_tokenized) const {
  pre_tokenized =
      std::accumulate(pretokenizers.begin(), pretokenizers.end(), pre_tokenized,
                      [](const PreTokenizedString& pre_tokenized,
                         const std::unique_ptr<PreTokenizer>& pre_tokenizer) {
                        return pre_tokenizer->pre_tokenize(pre_tokenized);
                      });
  return pre_tokenized;
}

SplitPreTokenizer::SplitPreTokenizer(std::string pattern, std::string behavior,
                                     bool invert)
    : pattern(pattern),
      behavior(get_split_delimiter_behavior(behavior)),
      invert(invert) {}

PreTokenizedString SplitPreTokenizer::pre_tokenize(
    PreTokenizedString pre_tokenized) const {}

ByteLevelPreTokenizer::ByteLevelPreTokenizer(bool add_prefix_space,
                                             bool trim_offsets, bool use_regex)
    : add_prefix_space(add_prefix_space),
      trim_offsets(trim_offsets),
      use_regex(use_regex) {}

PreTokenizedString ByteLevelPreTokenizer::pre_tokenize(
    PreTokenizedString pre_tokenized) const {}
