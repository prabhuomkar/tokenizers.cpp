// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/normalizer.h"

#include <gtest/gtest.h>

TEST(BertNormalizerTest, CleanText) {
  BertNormalizer normalizer(true, false, false, false);
  std::wstring got = normalizer.normalize(L"Hello World!\tThis is a test.\n");
  EXPECT_EQ(L"Hello World! This is a test. ", got);
}

TEST(BertNormalizerTest, HandleChineseChars) {
  BertNormalizer normalizer(false, true, false, false);
  std::wstring got = normalizer.normalize(L"Hello 世界!");
  EXPECT_EQ(L"Hello  世  界 !", got);
}

TEST(BertNormalizerTest, StripAccents) {
  BertNormalizer normalizer(false, false, true, false);
  std::wstring got = normalizer.normalize(L"Hélló Wórld! Thïs ïs á tést.");
  EXPECT_EQ(L"Hello World! This is a test.", got);
}

TEST(BertNormalizerTest, Lowercase) {
  BertNormalizer normalizer(false, false, false, true);
  std::wstring got = normalizer.normalize(L"HELLO WORLD! THIS IS A TEST!");
  EXPECT_EQ(L"hello world! this is a test!", got);
}
