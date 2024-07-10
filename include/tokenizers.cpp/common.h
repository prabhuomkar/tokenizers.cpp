// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>
#include <utility>
#include <vector>

class Encoding {
 public:
  std::vector<int> ids;
  std::vector<int> type_ids;
  std::vector<std::string> tokens;
  std::vector<int> words;
  std::vector<int> special_tokens_mask;
  std::vector<int> attention_mask;
};

class Token {
 public:
  int id;
  std::string value;
  std::pair<int, int> offsets;
  Token();
  Token(int id, std::string value, std::pair<int, int> offsets);
};
