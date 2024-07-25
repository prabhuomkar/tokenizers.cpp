// Copyright 2024 Omkar Prabhu
#include "tokenizers/model.h"

#include <gtest/gtest.h>

#include <vector>

#include "simdjson.h"

std::unique_ptr<Model> get_model_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object model_params = doc.get_object().value();
  return with_model(model_params);
}

void assert_tokens(std::vector<Token> expected, std::vector<Token> got) {
  EXPECT_EQ(expected.size(), got.size());
  for (int i = 0; i < expected.size(); i++) {
    EXPECT_EQ(expected[i].id, got[i].id);
    EXPECT_EQ(expected[i].offsets, got[i].offsets);
    EXPECT_EQ(expected[i].value, got[i].value);
  }
}

TEST(WordPieceModelTest, EmptyVocab) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"WordPiece\",\"unk_token\":\"[UNK]\",\"continuing_subword_"
      "prefix\":\"##\",\"max_input_chars_per_word\":100,\"vocab\":null}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"hello world";
  std::vector<Token> expected = {Token(0, "[UNK]", {0, input.length()})};
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}

TEST(WordPieceModelTest, Vocab) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"WordPiece\",\"unk_token\":\"[UNK]\",\"continuing_subword_"
      "prefix\":\"##\",\"max_input_chars_per_word\":100,\"vocab\":{\"hello\":"
      "420,\"[UNK]\":333}}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"hello world";
  std::vector<Token> expected = {
      Token(333, "[UNK]", {0, input.length()}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}

TEST(WordPieceModelTest, UNKToken) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"WordPiece\",\"unk_token\":\"[TEST_UNK]\",\"continuing_"
      "subword_prefix\":\"##\",\"max_input_chars_per_word\":100,\"vocab\":{\"["
      "TEST_UNK]\":333}}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"hello world";
  std::vector<Token> expected = {
      Token(333, "[TEST_UNK]", {0, input.length()}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}

TEST(WordPieceModelTest, MaxInputCharsPerWord) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"WordPiece\",\"unk_token\":\"[UNK]\",\"continuing_subword_"
      "prefix\":\"##\",\"max_input_chars_per_word\":10,\"vocab\":{\"[UNK]\":"
      "333}}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"hello world";
  std::vector<Token> expected = {
      Token(333, "[UNK]", {0, input.length()}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}

TEST(WordPieceModelTest, ContinuingSubWordPrefix) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"WordPiece\",\"unk_token\":\"[UNK]\",\"continuing_subword_"
      "prefix\":\"TEST_##\",\"max_input_chars_per_word\":100,\"vocab\":{"
      "\"hello\":420,\"TEST_## world\":111},\"[UNK]\":333}}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"hello world";
  std::vector<Token> expected = {
      Token(420, "hello", {0, 5}),
      Token(111, "TEST_## world", {5, 11}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}
