// Copyright 2024 Omkar Prabhu
#pragma once

#include <codecvt>
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

class Split {
 public:
  std::string normalized;
  std::pair<int, int> offsets;
  std::vector<Token> tokens;
  Split();
  Split(std::string normalized, std::pair<int, int> offsets);
};

std::string convert_to_string(std::wstring sequence);

std::wstring convert_from_string(std::string sequence);
