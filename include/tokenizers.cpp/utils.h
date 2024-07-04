// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.h"

class Truncation {
 public:
  Truncation();

 private:
  std::string direction;
  std::string strategy;
  int max_length;
  int stride;
};

class Padding {
 public:
  Padding();

 private:
  std::string direction;
  std::string strategy;
  int pad_id;
  int pad_type_id;
  std::string pad_token;
  int pad_to_multiple_of;
};

class AddedToken {
 public:
  int id;
  std::string_view content;
  bool single_word;
  bool lstrip;
  bool rstrip;
  bool normalized;
  bool special;
  AddedToken();
  explicit AddedToken(int id, std::string_view content = "",
                      bool single_word = false, bool lstrip = false,
                      bool rstrip = false, bool normalized = true,
                      bool special = false);
};

class AddedVocabulary {
 public:
  explicit AddedVocabulary(std::vector<AddedToken> added_tokens = {});

 private:
  std::vector<AddedToken> added_tokens;
};

class AddedVocabularyConfig {
 public:
  std::vector<AddedToken> added_tokens;
  explicit AddedVocabularyConfig(simdjson::ondemand::array added_tokens_params);
};

std::unordered_map<std::string_view, int> get_map_ints_from_json(
    simdjson::ondemand::object json_object);
