// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/model.h"

#include <gtest/gtest.h>

void assertTokens(std::vector<Token> expected, std::vector<Token> got) {
  EXPECT_EQ(expected.size(), got.size());
  for (int i = 0; i < expected.size(); i++) {
    EXPECT_EQ(expected[i].id, got[i].id);
    EXPECT_EQ(expected[i].offsets, got[i].offsets);
    EXPECT_EQ(expected[i].value, got[i].value);
  }
}

TEST(WordPieceModelTest, EmptyVocab) {
  static std::string input = "hello world";
  WordPiece model({});
  std::vector<Token> expected = {Token(0, "[UNK]", {0, input.length()})};
  std::vector<Token> got = model.tokenize(input);
  assertTokens(expected, got);
}

TEST(WordPieceModelTest, Vocab) {
  static std::string input = "hello world";
  WordPiece model({{"hello", 420}, {"[UNK]", 333}});
  std::vector<Token> expected = {
      Token(333, "[UNK]", {0, input.length()}),
  };
  std::vector<Token> got = model.tokenize(input);
  assertTokens(expected, got);
}

TEST(WordPieceModelTest, UNKToken) {
  static std::string input = "hello world";
  WordPiece model({{"[TEST_UNK]", 333}}, "[TEST_UNK]");
  std::vector<Token> expected = {
      Token(333, "[TEST_UNK]", {0, input.length()}),
  };
  std::vector<Token> got = model.tokenize(input);
  assertTokens(expected, got);
}

TEST(WordPieceModelTest, MaxInputCharsPerWord) {
  static std::string input = "hello world";
  WordPiece model({{"[UNK]", 333}}, "[UNK]", 10);
  std::vector<Token> expected = {
      Token(333, "[UNK]", {0, input.length()}),
  };
  std::vector<Token> got = model.tokenize(input);
  assertTokens(expected, got);
}

TEST(WordPieceModelTest, ContinuingSubWordPrefix) {
  static std::string input = "hello world";
  WordPiece model({{"hello", 420}, {"TEST_## world", 111}, {"[UNK]", 333}},
                  "[UNK]", 100, "TEST_##");
  std::vector<Token> expected = {
      Token(420, "hello", {0, 5}),
      Token(111, "TEST_## world", {5, 11}),
  };
  std::vector<Token> got = model.tokenize(input);
  assertTokens(expected, got);
}
