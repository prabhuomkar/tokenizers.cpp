// Copyright 2024 Omkar Prabhu
#include "tokenizers/common.h"

#include <gtest/gtest.h>

#include <string>

TEST(CommonTest, ConvertToString) {
  std::string expected = "Hello, 世界!";
  std::string got = convert_to_string(L"Hello, 世界!");
  EXPECT_EQ(expected, got);
}

TEST(CommonTest, ConvertFromString) {
  std::wstring expected = L"Hello, 世界!";
  std::wstring got = convert_from_string("Hello, 世界!");
  EXPECT_EQ(expected, got);
}
