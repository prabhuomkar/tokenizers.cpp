// Copyright 2024 Omkar Prabhu
#include "tokenizers/pre_tokenizer.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

std::unique_ptr<PreTokenizer> get_pre_tokenizer_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object pre_tokenizer_params = doc.get_object().value();
  return with_pre_tokenizer(pre_tokenizer_params);
}

void validate_splits(std::vector<Split> expected, std::vector<Split> got) {
  EXPECT_EQ(expected.size(), got.size());
  for (int i = 0; i < expected.size(); i++) {
    EXPECT_EQ(expected[i].normalized, got[i].normalized);
    EXPECT_EQ(expected[i].offsets.first, got[i].offsets.first);
    EXPECT_EQ(expected[i].offsets.second, got[i].offsets.second);
  }
}

TEST(BertPreTokenizerTest, Simple) {
  std::unique_ptr<PreTokenizer> pre_tokenizer = get_pre_tokenizer_from_string(
      "{\"type\":\"BertPreTokenizer\",\"clean_text\":true,\"handle_chinese_"
      "chars\":false,\"strip_accents\":null,\"lowercase\":false}");
  EXPECT_NE(pre_tokenizer, nullptr);
  std::vector<Split> expected = {
      Split("Hey", {0, 3}),   Split("friend", {4, 10}), Split("!", {10, 11}),
      Split("How", {16, 19}), Split("are", {20, 23}),   Split("you", {24, 27}),
      Split("?", {27, 28}),   Split("!", {28, 29}),     Split("?", {29, 30})};
  auto got = pre_tokenizer->pre_tokenize(
      PreTokenizedString(NormalizedString(L"Hey friend!     How are you?!?")));
  validate_splits(expected, got.splits);
  expected = {Split("野", {0, 1}),       Split("口", {2, 3}),
              Split("里", {4, 5}),       Split("佳", {6, 7}),
              Split("Noguchi", {8, 15}), Split("Rika", {16, 20})};
  got = pre_tokenizer->pre_tokenize(
      PreTokenizedString(NormalizedString(L"野 口 里 佳 Noguchi Rika")));
  validate_splits(expected, got.splits);
}

TEST(SplitPreTokenizerTest, Simple) {
  std::unique_ptr<PreTokenizer> pre_tokenizer = get_pre_tokenizer_from_string(
      "{\"type\":\"Split\",\"pattern\":{\"Regex\":\"(?i:'s|'t|'re|'ve|'m|'ll|'"
      "d)|[^\\\\r\\\\n\\\\p{L}\\\\p{N}]?\\\\p{L}+|\\\\p{N}{1,3}| "
      "?[^\\\\s\\\\p{L}\\\\p{N}]+[\\\\r\\\\n]*|\\\\s*[\\\\r\\\\n]+|\\\\s+(?!"
      "\\\\S)|\\\\s+\"},\"behavior\":\"Isolated\",\"invert\":false}");
  EXPECT_NE(pre_tokenizer, nullptr);
  std::vector<Split> expected = {
      Split("Why", {0, 3}),     Split(" don", {3, 7}),
      Split("'t", {7, 9}),      Split(" you", {9, 13}),
      Split(" give", {13, 18}), Split(" ", {18, 19}),
      Split("100", {19, 22}),   Split(" dollars", {22, 30}),
      Split("?", {30, 31})};
  auto got = pre_tokenizer->pre_tokenize(
      PreTokenizedString(NormalizedString(L"Why don't you give 100 dollars?")));
  validate_splits(expected, got.splits);
}

TEST(ByteLevelPreTokenizerTest, Simple) {
  std::unique_ptr<PreTokenizer> pre_tokenizer = get_pre_tokenizer_from_string(
      "{\"type\":\"ByteLevel\",\"add_prefix_space\":false,\"use_regex\":"
      "false}");
  EXPECT_NE(pre_tokenizer, nullptr);
  std::vector<Split> expected = {Split("HowĠareĠyaĠdoing?", {0, 17})};
  auto got = pre_tokenizer->pre_tokenize(
      PreTokenizedString(NormalizedString(L"How are ya doing?")));
  validate_splits(expected, got.splits);
}

TEST(ByteLevelPreTokenizerTest, AddPrefixSpace) {
  std::unique_ptr<PreTokenizer> pre_tokenizer = get_pre_tokenizer_from_string(
      "{\"type\":\"ByteLevel\",\"add_prefix_space\":true,\"use_regex\":false}");
  EXPECT_NE(pre_tokenizer, nullptr);
  std::vector<Split> expected = {Split("ĠHowĠareĠyaĠdoing?", {0, 18})};
  auto got = pre_tokenizer->pre_tokenize(
      PreTokenizedString(NormalizedString(L"How are ya doing?")));
  validate_splits(expected, got.splits);
}

TEST(SequencePreTokenizerTest, Simple) {
  std::unique_ptr<PreTokenizer> pre_tokenizer = get_pre_tokenizer_from_string(
      "{\"type\":\"Sequence\",\"pretokenizers\":[{\"type\":\"Split\","
      "\"pattern\":{\"Regex\":\"(?i:'s|'t|'re|'ve|'m|'ll|'d)|[^\\\\r\\\\n\\\\p{"
      "L}\\\\p{N}]?\\\\p{L}+|\\\\p{N}{1,3}| "
      "?[^\\\\s\\\\p{L}\\\\p{N}]+[\\\\r\\\\n]*|\\\\s*[\\\\r\\\\n]+|\\\\s+(?!"
      "\\\\S)|\\\\s+\"},\"behavior\":\"Isolated\",\"invert\":false},{\"type\":"
      "\"ByteLevel\",\"add_prefix_space\":false,\"trim_offsets\":true,\"use_"
      "regex\":false}]}");
  EXPECT_NE(pre_tokenizer, nullptr);
  std::vector<Split> expected = {
      Split("How", {0, 3}), Split("Ġare", {3, 7}), Split("Ġya", {7, 10}),
      Split("Ġdoing", {10, 16}), Split("?", {16, 17})};
  auto got = pre_tokenizer->pre_tokenize(
      PreTokenizedString(NormalizedString(L"How are ya doing?")));
  validate_splits(expected, got.splits);
}
