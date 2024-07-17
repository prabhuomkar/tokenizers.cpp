// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"

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
  int add_tokens(std::vector<AddedToken> tokens, std::unique_ptr<Model> model,
                 std::unique_ptr<Normalizer> normalizer);
  int add_special_tokens(std::vector<AddedToken> tokens,
                         std::unique_ptr<Model> model,
                         std::unique_ptr<Normalizer> normalizer);

 private:
  std::unordered_map<std::string, int> added_tokens_map;
  std::unordered_map<int, AddedToken> added_tokens_map_r;
  std::vector<AddedToken> special_tokens;
  std::unordered_set<std::string> special_tokens_set;
  std::pair<std::vector<std::string>, std::vector<int>>
      split_non_normalized_trie;
  std::pair<std::vector<std::string>, std::vector<int>> split_normalized_trie;
  void refresh_added_tokens(std::unique_ptr<Model> model,
                            std::unique_ptr<Normalizer> normalizer);
  std::vector<std::pair<
      std::string,
      std::optional<std::tuple<int, std::string, std::pair<int, int>>>>>
  find_matches(std::string sentence,
               std::pair<std::vector<std::string>, std::vector<int>> split_re);
};

class AddedVocabularyConfig {
 public:
  std::vector<AddedToken> added_tokens;
  explicit AddedVocabularyConfig(simdjson::ondemand::array added_tokens_params);
};
