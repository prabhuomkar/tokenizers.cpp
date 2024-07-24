// Copyright 2024 Omkar Prabhu
#include "tokenizers/post_processor.h"

#include <gtest/gtest.h>

#include "simdjson.h"

std::unique_ptr<PostProcessor> get_post_processor_from_string(
    std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object post_processor_params = doc.get_object().value();
  return with_post_processor(post_processor_params);
}

void assert_post_processor_encoding(Encoding expected, Encoding got) {
  EXPECT_EQ(expected.ids, got.ids);
  EXPECT_EQ(expected.offsets, got.offsets);
  EXPECT_EQ(expected.tokens, got.tokens);
  EXPECT_EQ(expected.type_ids, got.type_ids);
  EXPECT_EQ(expected.words, got.words);
  EXPECT_EQ(expected.special_tokens_mask, got.special_tokens_mask);
  EXPECT_EQ(expected.attention_mask, got.attention_mask);
}

TEST(TemplateProcessingTest, AddSpecialTokens) {
  std::unique_ptr<PostProcessor> post_processor =
      get_post_processor_from_string(
          "{\"type\":\"TemplateProcessing\",\"single\":[{\"SpecialToken\":{"
          "\"id\":\"[CLS]\",\"type_id\":0}},{\"Sequence\":{\"id\":\"A\",\"type_"
          "id\":0}},{\"SpecialToken\":{\"id\":\"[SEP]\",\"type_id\":0}}],"
          "\"pair\":[{\"SpecialToken\":{\"id\":\"[CLS]\",\"type_id\":0}},{"
          "\"Sequence\":{\"id\":\"A\",\"type_id\":0}},{\"SpecialToken\":{"
          "\"id\":\"[SEP]\",\"type_id\":0}},{\"Sequence\":{\"id\":\"B\",\"type_"
          "id\":1}},{\"SpecialToken\":{\"id\":\"[SEP]\",\"type_id\":1}}],"
          "\"special_tokens\":{\"[CLS]\":{\"id\":\"[CLS]\",\"ids\":[101],"
          "\"tokens\":[\"[CLS]\"]},\"[SEP]\":{\"id\":\"[SEP]\",\"ids\":[102],"
          "\"tokens\":[\"[SEP]\"]}}}");
  EXPECT_NE(post_processor, nullptr);
  Encoding input_encoding({12, 14}, {0, 0}, {"hello", "world"}, {0, 0},
                          {{0, 5}, {6, 11}}, {0, 0}, {1, 1});
  Encoding expected(
      {101, 12, 14, 102}, {0, 0, 0, 0}, {"[CLS]", "hello", "world", "[SEP]"},
      {std::nullopt, 0, 0, std::nullopt}, {{0, 0}, {0, 5}, {6, 11}, {0, 0}},
      {1, 0, 0, 1}, {1, 1, 1, 1});
  Encoding got = post_processor->process(input_encoding, true);
  assert_post_processor_encoding(expected, got);
}

TEST(TemplateProcessingTest, WithoutSpecialTokens) {
  std::unique_ptr<PostProcessor> post_processor =
      get_post_processor_from_string(
          "{\"type\":\"TemplateProcessing\",\"single\":[{\"SpecialToken\":{"
          "\"id\":\"[CLS]\",\"type_id\":0}},{\"Sequence\":{\"id\":\"A\",\"type_"
          "id\":0}},{\"SpecialToken\":{\"id\":\"[SEP]\",\"type_id\":0}}],"
          "\"pair\":[{\"SpecialToken\":{\"id\":\"[CLS]\",\"type_id\":0}},{"
          "\"Sequence\":{\"id\":\"A\",\"type_id\":0}},{\"SpecialToken\":{"
          "\"id\":\"[SEP]\",\"type_id\":0}},{\"Sequence\":{\"id\":\"B\",\"type_"
          "id\":1}},{\"SpecialToken\":{\"id\":\"[SEP]\",\"type_id\":1}}],"
          "\"special_tokens\":{\"[CLS]\":{\"id\":\"[CLS]\",\"ids\":[101],"
          "\"tokens\":[\"[CLS]\"]},\"[SEP]\":{\"id\":\"[SEP]\",\"ids\":[102],"
          "\"tokens\":[\"[SEP]\"]}}}");
  EXPECT_NE(post_processor, nullptr);
  Encoding input_encoding({12, 14}, {0, 0}, {"hello", "world"}, {0, 0},
                          {{0, 5}, {6, 11}}, {0, 0}, {1, 1});
  Encoding expected({12, 14}, {0, 0}, {"hello", "world"}, {0, 0},
                    {{0, 5}, {6, 11}}, {0, 0}, {1, 1});
  Encoding got = post_processor->process(input_encoding, false);
  assert_post_processor_encoding(expected, got);
}
