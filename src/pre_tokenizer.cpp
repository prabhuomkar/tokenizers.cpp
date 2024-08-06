// Copyright 2024 Omkar Prabhu
#include "tokenizers/pre_tokenizer.h"

#include <unicode/regex.h>
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
#include <utility>
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
    val = pre_tokenizer_params["use_regex"].value();
    bool use_regex = val.type() == simdjson::ondemand::json_type::null
                         ? false
                         : static_cast<bool>(val.get_bool());
    return std::make_unique<ByteLevelPreTokenizer>(
        ByteLevelPreTokenizer(add_prefix_space, use_regex));
  }
  return nullptr;
}

std::vector<std::pair<std::pair<int, int>, bool>> find_matches(
    std::string regex, icu::UnicodeString input) {
  std::vector<std::pair<int, int>> matches;
  UErrorCode status = U_ZERO_ERROR;
  icu::RegexPattern* unicode_pattern = icu::RegexPattern::compile(
      icu::UnicodeString::fromUTF8(regex), 0, status);
  icu::RegexMatcher* matcher = unicode_pattern->matcher(input, status);
  while (matcher->find(status)) {
    matches.push_back({matcher->start(status), matcher->end(status)});
  }
  delete matcher;
  delete unicode_pattern;

  std::vector<std::pair<std::pair<int, int>, bool>> result;
  int cur = 0;
  for (auto match : matches) {
    if (cur != match.first) {
      result.push_back({{cur, match.first}, false});
    }
    result.push_back({match, true});
    cur = match.second;
  }
  if (cur < input.length()) {
    result.push_back({{cur, input.length()}, false});
  }
  return result;
}

std::vector<std::pair<std::pair<int, int>, bool>> is_whitespace(
    icu::UnicodeString input) {
  std::string pattern = "\\s+";
  return find_matches(pattern, input);
}

std::vector<std::pair<std::pair<int, int>, bool>> is_bert_punc(
    icu::UnicodeString input) {
  std::string pattern = "\\p{P}";
  return find_matches(pattern, input);
}

std::vector<Split> split_normalized(
    Split original_split,
    std::function<
        std::vector<std::pair<std::pair<int, int>, bool>>(icu::UnicodeString)>
        split_fn,
    SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::vector<Split> new_splits;
  icu::UnicodeString unicode_normalized =
      icu::UnicodeString::fromUTF8(original_split.normalized);
  std::vector<std::pair<std::pair<int, int>, bool>> matches =
      split_fn(unicode_normalized);
  switch (pattern) {
    case REMOVED:
      break;
    case ISOLATED:
      for (auto& match : matches) {
        match.second = false;
      }
      break;
  }
  for (auto match : matches) {
    if (!match.second) {
      std::string split_unicode_normalized;
      unicode_normalized
          .tempSubStringBetween(match.first.first, match.first.second)
          .toUTF8String(split_unicode_normalized);
      new_splits.push_back(
          Split(split_unicode_normalized,
                {match.first.first + original_split.offsets.first,
                 match.first.second + original_split.offsets.first}));
    }
  }
  return new_splits;
}

void PreTokenizedString::split(
    std::function<
        std::vector<std::pair<std::pair<int, int>, bool>>(icu::UnicodeString)>
        split_fn,
    SPLIT_DELIMITER_BEHAVIOR pattern) {
  std::vector<Split> new_splits;
  for (auto orig_split : splits) {
    if (orig_split.tokens.size() != 0) {
      new_splits.push_back(orig_split);
      continue;
    }

    std::vector<Split> new_normalized_splits =
        split_normalized(orig_split, split_fn, pattern);
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

SplitPreTokenizer::SplitPreTokenizer(const std::string& pattern,
                                     const std::string& behavior, bool invert)
    : pattern(pattern),
      behavior(get_split_delimiter_behavior(behavior)),
      invert(invert) {}

PreTokenizedString SplitPreTokenizer::pre_tokenize(
    PreTokenizedString pre_tokenized) const {
  auto matches_regex = [this](icu::UnicodeString input)
      -> std::vector<std::pair<std::pair<int, int>, bool>> {
    return find_matches(pattern, input);
  };

  pre_tokenized.split(matches_regex, behavior);
  return pre_tokenized;
}

ByteLevelPreTokenizer::ByteLevelPreTokenizer(bool add_prefix_space,
                                             bool use_regex)
    : add_prefix_space(add_prefix_space),
      use_regex(use_regex),
      regex(
          R"('s|'t|'re|'ve|'m|'ll|'d| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+)",
          std::regex_constants::optimize) {
  std::vector<uint16_t> bs;
  for (uint16_t i = '!'; i <= '~'; ++i) {
    bs.push_back(i);
  }
  for (uint16_t i = 0xA1; i <= 0xAC; ++i) {
    bs.push_back(i);
  }
  for (uint16_t i = 0xAE; i <= 0xFF; ++i) {
    bs.push_back(i);
  }
  std::vector<uint32_t> cs;
  std::transform(bs.begin(), bs.end(), std::back_inserter(cs),
                 [](uint16_t b) { return static_cast<uint32_t>(b); });
  uint32_t n = 0;
  for (uint16_t b = 0; b <= 255; ++b) {
    if (std::find(bs.begin(), bs.end(), b) == bs.end()) {
      bs.push_back(b);
      cs.push_back((1 << 8) + n);
      ++n;
    }
  }
  for (size_t i = 0; i < bs.size(); ++i) {
    uint16_t byte = bs[i];
    uint32_t code_point = cs[i];
    if (code_point <= 0x10FFFF) {
      icu::UnicodeString unicode_str;
      if (code_point <= 0xFFFF) {
        unicode_str.append(static_cast<UChar>(code_point));
      } else {
        code_point -= 0x10000;
        unicode_str.append(static_cast<UChar>((code_point >> 10) + 0xD800));
        unicode_str.append(static_cast<UChar>((code_point & 0x3FF) + 0xDC00));
      }
      bytes_char[byte] = unicode_str;
    }
  }
}

PreTokenizedString ByteLevelPreTokenizer::pre_tokenize(
    PreTokenizedString pre_tokenized) const {
  if (add_prefix_space &&
      !std::iswspace(pre_tokenized.normalized.normalized.at(0))) {
    pre_tokenized.normalized.normalized =
        std::wstring(L" ") + pre_tokenized.normalized.normalized;
    pre_tokenized.normalized.transform(0, "add", std::wstring(L" ").length());
    pre_tokenized = PreTokenizedString(pre_tokenized.normalized);
  }
  auto matches_regex = [this](icu::UnicodeString input)
      -> std::vector<std::pair<std::pair<int, int>, bool>> {
    if (use_regex) {
      return find_matches(regex, input);
    }
    return {{{0, input.length()}, false}};
  };
  pre_tokenized.split(matches_regex, SPLIT_DELIMITER_BEHAVIOR::ISOLATED);
  for (Split& split : pre_tokenized.splits) {
    std::string new_split_normalized;
    for (const char c : split.normalized) {
      std::string schar;
      bytes_char.at(static_cast<int>(c)).toUTF8String(schar);
      new_split_normalized += schar;
    }
    split.normalized = new_split_normalized;
  }
  return pre_tokenized;
}
