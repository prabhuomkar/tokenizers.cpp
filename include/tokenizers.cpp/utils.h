// Copyright 2024 Omkar Prabhu
#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "simdjson.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"

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
  std::string content;
  bool single_word;
  bool lstrip;
  bool rstrip;
  bool normalized;
  bool special;
  AddedToken();
  explicit AddedToken(int id, std::string content = "",
                      bool single_word = false, bool lstrip = false,
                      bool rstrip = false, bool normalized = true,
                      bool special = false);
};

class AddedVocabulary {
 public:
  std::vector<AddedToken> added_tokens;
  AddedVocabulary();
  explicit AddedVocabulary(std::vector<AddedToken> added_tokens);
  int add_tokens(std::vector<AddedToken> tokens, Model model,
                 std::optional<Normalizer> normalizer);
  int add_special_tokens(std::vector<AddedToken> tokens, Model model,
                         std::optional<Normalizer> normalizer);

 private:
  std::unordered_map<std::string, int> added_tokens_map;
  std::unordered_map<int, AddedToken> added_tokens_map_r;
  std::vector<AddedToken> special_tokens;
  std::unordered_set<std::string> special_tokens_set;
  void refresh_added_tokens(Model model, std::optional<Normalizer> normalizer);
};

class AddedVocabularyConfig {
 public:
  std::vector<AddedToken> added_tokens;
  explicit AddedVocabularyConfig(simdjson::ondemand::array added_tokens_params);
};

std::unordered_map<std::string, int> get_map_ints_from_json(
    simdjson::ondemand::object json_object);
