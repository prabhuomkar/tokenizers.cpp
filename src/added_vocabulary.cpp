// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/added_vocabulary.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"

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

int AddedVocabulary::add_tokens(std::vector<AddedToken> tokens, Model model,
                                std::unique_ptr<Normalizer> normalizer) {
  for (auto token : tokens) {
    if (token.special && !token.content.empty() &&
        special_tokens_set.count(token.content) != 1) {
      special_tokens.push_back(token);
      special_tokens_set.insert(token.content);
    }
  }

  int ignored = 0;
  for (auto token : tokens) {
    if (token.content.empty()) {
      ignored++;
      continue;
    }

    int new_id;
    auto it = added_tokens_map.find(token.content);
    if (it != added_tokens_map.end()) {
      new_id = it->second;
    } else if (model.token_to_id(token.content).has_value()) {
      new_id = model.token_to_id(token.content).value();
    } else {
      auto max_val_token =
          std::max_element(added_tokens_map.begin(), added_tokens_map.end(),
                           [](const std::pair<std::string, int>& a,
                              const std::pair<std::string, int>& b) {
                             return a.second < b.second;
                           });
      if (model.get_vocab_size() == 0 ||
          max_val_token->second >= model.get_vocab_size()) {
        new_id = max_val_token->second + 1;
      } else {
        new_id = model.get_vocab_size();
      }
    }
    added_tokens_map[token.content] = new_id;
    added_tokens_map_r[new_id] = token;

    if (!special_tokens_set.count(token.content)) {
      added_tokens.push_back(token);
    }
  }

  refresh_added_tokens(model, std::move(normalizer));

  return tokens.size() - ignored;
}

void AddedVocabulary::refresh_added_tokens(
    Model model, std::unique_ptr<Normalizer> normalizer) {
  std::vector<std::pair<AddedToken, int>> normalized, non_normalized;
  for (auto token : special_tokens) {
    int id = model.token_to_id(token.content).value();
    if (token.normalized) {
      normalized.push_back({token, id});
    } else {
      non_normalized.push_back({token, id});
    }
  }
  for (auto token : added_tokens) {
    int id = model.token_to_id(token.content).value();
    if (token.normalized) {
      normalized.push_back({token, id});
    } else {
      non_normalized.push_back({token, id});
    }
  }

  // TODO(omkar): build trie to do better pattern matching
  std::vector<std::string> normalized_tokens;
  std::vector<int> normalized_ids;
  for (auto element : normalized) {
    normalized_tokens.push_back(element.first.content);
    normalized_ids.push_back(element.second);
  }
  split_normalized_trie = {normalized_tokens, normalized_ids};
  std::vector<std::string> non_normalized_tokens;
  std::vector<int> non_normalized_ids;
  for (auto element : non_normalized) {
    non_normalized_tokens.push_back(element.first.content);
    non_normalized_ids.push_back(element.second);
  }
  split_non_normalized_trie = {non_normalized_tokens, non_normalized_ids};
}

std::vector<
    std::pair<std::string,
              std::optional<std::tuple<int, std::string, std::pair<int, int>>>>>
AddedVocabulary::find_matches(
    std::string sentence,
    std::pair<std::vector<std::string>, std::vector<int>> split_re) {
  // TODO(omkar): update to find matches from trie
  std::vector<std::pair<
      std::string,
      std::optional<std::tuple<int, std::string, std::pair<int, int>>>>>
      result;
  std::unordered_map<std::string, int> word_ids;
  for (size_t i = 0; i < split_re.first.size(); ++i) {
    word_ids[split_re.first[i]] = split_re.second[i];
  }
  size_t start = 0;
  for (size_t i = 0; i < sentence.length(); ++i) {
    if (sentence[i] == ' ') {
      if (i > start && word_ids.count(sentence.substr(start, i - start)) > 0) {
        std::string word = sentence.substr(start, i - start);
        result.push_back({word, std::make_tuple(word_ids[word], word,
                                                std::make_pair(start, i))});
      }
      start = i + 1;
    }
  }
  if (start < sentence.length() && word_ids.count(sentence.substr(start)) > 0) {
    std::string word = sentence.substr(start);
    result.push_back(
        {word, std::make_tuple(word_ids[word], word,
                               std::make_pair(start, sentence.length()))});
  }
  return result;
}

int AddedVocabulary::add_special_tokens(
    std::vector<AddedToken> tokens, Model model,
    std::unique_ptr<Normalizer> normalizer) {
  return add_tokens(tokens, model, std::move(normalizer));
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
