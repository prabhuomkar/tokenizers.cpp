// Copyright 2024 Omkar Prabhu
#include "tokenizers/pre_tokenizer.h"

#include <gtest/gtest.h>

#include <memory>
#include <typeinfo>
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
  std::vector<Split> got =
      pre_tokenizer->pre_tokenize(L"Hey friend!     How are you?!?");
  validate_splits(expected, got);
  expected = {Split("野", {0, 1}),       Split("口", {2, 3}),
              Split("里", {4, 5}),       Split("佳", {6, 7}),
              Split("Noguchi", {8, 15}), Split("Rika", {16, 20})};
  got = pre_tokenizer->pre_tokenize(L"野 口 里 佳 Noguchi Rika");
  validate_splits(expected, got);
}
