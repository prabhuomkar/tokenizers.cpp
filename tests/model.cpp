// Copyright 2024 Omkar Prabhu
#include "tokenizers/model.h"

#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

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

TEST(BPEModelTest, UNKTokenNotFused) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"BPE\",\"dropout\":null,\"unk_token\":\"<unk>\","
      "\"continuing_subword_prefix\":null,\"end_of_word_suffix\":null,\"fuse_"
      "unk\":false,\"byte_fallback\":false,\"ignore_merges\":true,\"vocab\":{"
      "\"<unk>\": 0,\"a\":1,\"b\":2},"
      "\"merges\":[]}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"c";
  std::vector<Token> expected = {
      Token(0, "<unk>", {0, 1}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
  input = L"cc";
  expected = {
      Token(0, "<unk>", {0, 1}),
      Token(0, "<unk>", {1, 2}),
  };
  got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}

TEST(BPEModelTest, UNKTokenFused) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"BPE\",\"dropout\":null,\"unk_token\":\"<unk>\","
      "\"continuing_subword_prefix\":null,\"end_of_word_suffix\":null,\"fuse_"
      "unk\":true,\"byte_fallback\":false,\"ignore_merges\":true,\"vocab\":{"
      "\"<unk>\": 0,\"a\":1,\"b\":2},"
      "\"merges\":[]}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"c";
  std::vector<Token> expected = {
      Token(0, "<unk>", {0, 1}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
  input = L"cc";
  expected = {
      Token(0, "<unk>", {0, 2}),
  };
  got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
  input = L"accb";
  expected = {
      Token(1, "a", {0, 1}),
      Token(0, "<unk>", {1, 3}),
      Token(2, "b", {3, 4}),
  };
  got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}

TEST(BPEModelTest, WithAndWithoutDropout) {
  std::unique_ptr<Model> model = get_model_from_string(
      "{\"type\":\"BPE\",\"dropout\":null,\"unk_token\":null,"
      "\"continuing_subword_prefix\":null,\"end_of_word_suffix\":null,\"fuse_"
      "unk\":false,\"byte_fallback\":false,\"ignore_merges\":false,\"vocab\":{"
      "\"u\":0,\"n\":1,\"r\":2,\"e\":3,\"l\":4,\"a\":5,\"t\":6,\"d\":7,\"re\":"
      "8,\"at\":9,\"ed\":10,\"un\":11,\"ated\":12,\"rel\":13,\"related\":14,"
      "\"unrelated\":15},\"merges\":[\"r e\",\"a t\",\"e d\",\"u n\",\"at "
      "ed\",\"re l\",\"rel ated\",\"un related\"]}");
  EXPECT_NE(model, nullptr);
  std::wstring input = L"unrelated";
  std::vector<Token> expected = {
      Token(15, "unrelated", {0, 9}),
  };
  auto got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
  model = get_model_from_string(
      "{\"type\":\"BPE\",\"dropout\":1.0,\"unk_token\":null,"
      "\"continuing_subword_prefix\":null,\"end_of_word_suffix\":null,\"fuse_"
      "unk\":false,\"byte_fallback\":false,\"ignore_merges\":true,\"vocab\":{"
      "\"u\":0,\"n\":1,\"r\":2,\"e\":3,\"l\":4,\"a\":5,\"t\":6,\"d\":7,\"re\":"
      "8,\"at\":9,\"ed\":10,\"un\":11,\"ated\":12,\"rel\":13,\"related\":14,"
      "\"unrelated\":15},\"merges\":[\"r e\",\"a t\",\"e d\",\"u n\",\"at "
      "ed\",\"re l\",\"rel ated\",\"un related\"]}");
  EXPECT_NE(model, nullptr);
  input = L"unrelated";
  expected = {
      Token(0, "u", {0, 1}), Token(1, "n", {1, 2}), Token(2, "r", {2, 3}),
      Token(3, "e", {3, 4}), Token(4, "l", {4, 5}), Token(5, "a", {5, 6}),
      Token(6, "t", {6, 7}), Token(3, "e", {7, 8}), Token(7, "d", {8, 9}),
  };
  got = model->tokenize(PreTokenizedString(NormalizedString(input)));
  assert_tokens(expected, got.splits[0].tokens);
}
