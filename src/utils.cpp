// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/utils.h"

#include <string>
#include <vector>

Truncation::Truncation() {}

Padding::Padding() {}

AddedToken::AddedToken() {}

AddedToken::AddedToken(int id, std::string content, bool single_word,
                       bool lstrip, bool rstrip, bool normalized, bool special)
    : id(id),
      content(content),
      single_word(single_word),
      lstrip(lstrip),
      rstrip(rstrip),
      normalized(normalized),
      special(special) {}

AddedVocabulary::AddedVocabulary() {}

AddedVocabulary::AddedVocabulary(std::vector<AddedToken> added_tokens)
    : added_tokens(added_tokens) {
  std::cout << "Initialized AddedVocabulary" << std::endl;
  std::cout << "added_tokens: ";
  for (const auto& added_token : added_tokens) {
    std::cout << "(" << added_token.id << "," << added_token.content << ","
              << added_token.single_word << "," << added_token.lstrip << ","
              << added_token.rstrip << "," << added_token.normalized << ","
              << added_token.special << ") ";
  }
  std::cout << std::endl;
}

AddedVocabularyConfig::AddedVocabularyConfig(
    simdjson::ondemand::array added_tokens_params) {
  added_tokens = {};
  for (simdjson::ondemand::value val : added_tokens_params) {
    simdjson::ondemand::object added_token_param = val.get_object();
    AddedToken added_token;
    for (auto element : added_token_param) {
      auto key = element.escaped_key();
      if (key.value() == "id") {
        added_token.id = static_cast<int>(element.value().get_int64());
      } else if (key.value() == "content") {
        added_token.content = std::string(
            static_cast<std::string_view>(element.value().get_string()));
      } else if (key.value() == "single_word") {
        added_token.single_word = static_cast<bool>(element.value().get_bool());
      } else if (key.value() == "lstrip") {
        added_token.lstrip = static_cast<bool>(element.value().get_bool());
      } else if (key.value() == "rstrip") {
        added_token.rstrip = static_cast<bool>(element.value().get_bool());
      } else if (key.value() == "normalized") {
        added_token.normalized = static_cast<bool>(element.value().get_bool());
      } else if (key.value() == "special") {
        added_token.special = static_cast<bool>(element.value().get_bool());
      }
    }
    added_tokens.push_back(added_token);
  }
}

std::unordered_map<std::string, int> get_map_ints_from_json(
    simdjson::ondemand::object json_object) {
  std::unordered_map<std::string, int> result;
  for (auto element : json_object) {
    result.insert(
        {std::string(static_cast<std::string_view>(element.escaped_key())),
         static_cast<int>(element.value().get_int64())});
  }
  return result;
}
