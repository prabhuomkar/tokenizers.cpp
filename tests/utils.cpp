// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/utils.h"

#include <gtest/gtest.h>

#include "simdjson.h"

std::unique_ptr<Truncation> get_truncation_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object truncation_params = doc.get_object().value();
  return with_truncation(truncation_params);
}

std::unique_ptr<Padding> get_padding_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object padding_params = doc.get_object().value();
  return with_padding(padding_params);
}

void assert_utils_encoding(Encoding expected, Encoding got) {
  EXPECT_EQ(expected.ids, got.ids);
  EXPECT_EQ(expected.offsets, got.offsets);
  EXPECT_EQ(expected.tokens, got.tokens);
  EXPECT_EQ(expected.type_ids, got.type_ids);
  EXPECT_EQ(expected.words, got.words);
  EXPECT_EQ(expected.special_tokens_mask, got.special_tokens_mask);
  EXPECT_EQ(expected.attention_mask, got.attention_mask);
  EXPECT_EQ(expected.overflowing.size(), got.overflowing.size());
  if (expected.overflowing.size() > 0) {
    for (int i = 0; i < expected.overflowing.size(); i++) {
      assert_utils_encoding(expected.overflowing[i], got.overflowing[i]);
    }
  }
}

TEST(TruncationTest, LongestFirst) {
  std::unique_ptr<Truncation> truncation = get_truncation_from_string(
      "{\"strategy\":\"LongestFirst\",\"direction\":\"Right\",\"max_length\":"
      "1,\"stride\":0}");
  EXPECT_NE(truncation, nullptr);
  Encoding input_encoding({12, 14}, {0, 0}, {"hello", "world"}, {0, 0},
                          {{0, 5}, {6, 11}}, {0, 0}, {1, 1});
  Encoding expected({12}, {0}, {"hello"}, {0}, {{0, 5}}, {0}, {1});
  expected.overflowing = {
      Encoding({14}, {0}, {"world"}, {0}, {{6, 11}}, {0}, {1})};
  Encoding got = truncation->truncate_encoding(input_encoding);
  assert_utils_encoding(expected, got);
}

TEST(TruncationTest, OnlyFirst) {
  std::unique_ptr<Truncation> truncation = get_truncation_from_string(
      "{\"strategy\":\"OnlyFirst\",\"direction\":\"Right\",\"max_length\":"
      "1,\"stride\":0}");
  EXPECT_NE(truncation, nullptr);
  Encoding input_encoding({12, 14}, {0, 0}, {"hello", "world"}, {0, 0},
                          {{0, 5}, {6, 11}}, {0, 0}, {1, 1});
  Encoding expected({12}, {0}, {"hello"}, {0}, {{0, 5}}, {0}, {1});
  expected.overflowing = {
      Encoding({14}, {0}, {"world"}, {0}, {{6, 11}}, {0}, {1})};
  Encoding got = truncation->truncate_encoding(input_encoding);
  assert_utils_encoding(expected, got);
}

TEST(PaddingTest, BatchLongest) {
  std::unique_ptr<Padding> padding = get_padding_from_string(
      "{\"strategy\":\"BatchLongest\",\"direction\":\"Right\",\"pad_id\":"
      "1,\"pad_type_id\":1,\"pad_token\":\"[PAD]\",\"pad_to_multiple_of\":3}");
  EXPECT_NE(padding, nullptr);
  Encoding input_encoding({12, 14, 16, 18}, {0, 0, 0, 0},
                          {"hello", "world", "from", "tokenizers"},
                          {0, 0, 0, 0}, {{0, 5}, {6, 11}, {12, 16}, {17, 27}},
                          {0, 0, 0, 0}, {1, 1, 1, 1});
  Encoding expected({12, 14, 16, 18, 1, 1}, {0, 0, 0, 0, 1, 1},
                    {"hello", "world", "from", "tokenizers", "[PAD]", "[PAD]"},
                    {0, 0, 0, 0, 0, 0},
                    {{0, 5}, {6, 11}, {12, 16}, {17, 27}, {0, 0}, {0, 0}},
                    {0, 0, 0, 0, 1, 1}, {1, 1, 1, 1, 0, 0});
  Encoding got = padding->pad_encoding(input_encoding);
  assert_utils_encoding(expected, got);
}

TEST(PaddingTest, Fixed) {
  std::unique_ptr<Padding> padding = get_padding_from_string(
      "{\"strategy\":{\"Fixed\":1},\"direction\":\"Right\",\"pad_id\":"
      "1,\"pad_type_id\":1,\"pad_token\":\"[PAD]\",\"pad_to_multiple_of\":3}");
  EXPECT_NE(padding, nullptr);
  Encoding input_encoding({12, 14}, {0, 0}, {"hello", "world"}, {0, 0},
                          {{0, 5}, {6, 11}}, {0, 0}, {1, 1});
  Encoding expected({12, 14, 1}, {0, 0, 1}, {"hello", "world", "[PAD]"},
                    {0, 0, 0}, {{0, 5}, {6, 11}, {0, 0}}, {0, 0, 1}, {1, 1, 0});
  Encoding got = padding->pad_encoding(input_encoding);
  assert_utils_encoding(expected, got);
}
