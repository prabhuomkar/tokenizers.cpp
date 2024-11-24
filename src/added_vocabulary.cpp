// Copyright 2024 Omkar Prabhu
#include "tokenizers/added_vocabulary.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tokenizers/model.h"
#include "tokenizers/normalizer.h"
#include "tokenizers/pre_tokenizer.h"

AddedToken::AddedToken()
    : id(0),
      content(""),
      single_word(false),
      lstrip(false),
      rstrip(false),
      normalized(true),
      special(false) {}

AddedToken::AddedToken(int id, std::string content, bool single_word,
                       bool lstrip, bool rstrip, bool normalized, bool special)
    : id(id),
      content(content),
      single_word(single_word),
      lstrip(lstrip),
      rstrip(rstrip),
      normalized(normalized),
      special(special) {}

AddedVocabulary::AddedVocabulary(std::vector<AddedToken> added_tokens)
    : added_tokens(added_tokens), encode_special_tokens(false) {}

bool AddedVocabulary::is_special_token(const std::string& token) const {
  return special_tokens_set.count(token) > 0;
}

std::optional<std::string> AddedVocabulary::id_to_token(int id) {
  auto it = added_tokens_map_r.find(id);
  if (it != added_tokens_map_r.end()) {
    return (it->second).content;
  }
  return std::nullopt;
}

int AddedVocabulary::add_tokens(const std::vector<AddedToken>& tokens,
                                Model* model, Normalizer* normalizer) {
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
    } else if (model->token_to_id(token.content).has_value()) {
      new_id = model->token_to_id(token.content).value();
    } else {
      auto max_val_token =
          std::max_element(added_tokens_map.begin(), added_tokens_map.end(),
                           [](const std::pair<std::string, int>& a,
                              const std::pair<std::string, int>& b) {
                             return a.second < b.second;
                           });
      if (model->get_vocab_size() == 0 ||
          (max_val_token != added_tokens_map.end() &&
           max_val_token->second >= model->get_vocab_size())) {
        new_id = max_val_token->second + 1;
      } else {
        new_id = model->get_vocab_size();
      }
    }
    added_tokens_map[token.content] = new_id;
    added_tokens_map_r[new_id] = token;

    if (!special_tokens_set.count(token.content)) {
      added_tokens.push_back(token);
    }
  }

  refresh_added_tokens(model, normalizer);

  return tokens.size() - ignored;
}

void AddedVocabulary::refresh_added_tokens(Model* model,
                                           Normalizer* normalizer) {
  std::vector<std::pair<AddedToken, int>> normalized, non_normalized;
  for (auto token : special_tokens) {
    auto id = model->token_to_id(token.content);
    if (id.has_value()) {
      if (token.normalized) {
        normalized.push_back({token, id.value()});
      } else {
        non_normalized.push_back({token, id.value()});
      }
    }
  }
  for (auto token : added_tokens) {
    auto id = model->token_to_id(token.content);
    if (id.has_value()) {
      if (token.normalized) {
        normalized.push_back({token, id.value()});
      } else {
        non_normalized.push_back({token, id.value()});
      }
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

bool ends_with_word(std::string sentence) {
  return std::regex_search(sentence, std::regex("\\w$"));
}

bool starts_with_word(std::string sentence) {
  return std::regex_search(sentence, std::regex("^\\w"));
}

size_t space_leftmost_at_end(std::string sentence) {
  std::smatch match;
  if (std::regex_search(sentence, match, std::regex("\\s*$"))) {
    return match.position();
  } else {
    return sentence.length();
  }
}

size_t space_rightmost_at_start(std::string sentence) {
  std::smatch match;
  if (std::regex_search(sentence, match, std::regex("^\\s*"))) {
    return match.length();
  } else {
    return 0;
  }
}

std::vector<std::pair<std::optional<int>, std::pair<int, int>>>
AddedVocabulary::find_matches(
    std::string sentence,
    std::pair<std::vector<std::string>, std::vector<int>> split_re) {
  // TODO(omkar): update to find matches from trie
  std::vector<std::tuple<int, int, int>> matches;
  std::unordered_map<std::string, int> word_ids;
  for (int i = 0; i < split_re.first.size(); ++i) {
    word_ids[split_re.first[i]] = split_re.second[i];
  }
  icu::UnicodeString unicode_sentence = icu::UnicodeString::fromUTF8(sentence);
  int i = 0, cur = 0;
  while (i < unicode_sentence.length()) {
    std::string sub_sentence;
    unicode_sentence.tempSubString(cur, i - cur).toUTF8String(sub_sentence);
    if (word_ids.count(sub_sentence) > 0) {
      matches.push_back({cur, i, word_ids[sub_sentence]});
      cur = i;
    }
    if (unicode_sentence[i] == ' ') {
      cur = i + 1;
    }
    i++;
  }
  if (i != cur) {
    std::string sub_sentence;
    unicode_sentence.tempSubString(cur, i - cur).toUTF8String(sub_sentence);
    if (word_ids.count(sub_sentence) > 0) {
      matches.push_back({cur, i, word_ids[sub_sentence]});
    }
  }
  std::vector<std::pair<std::optional<int>, std::pair<int, int>>> result;
  int start_offset = 0;
  for (auto match : matches) {
    int start = std::get<0>(match), stop = std::get<1>(match),
        id = std::get<2>(match);
    AddedToken added_token = added_tokens_map_r[id];
    if (encode_special_tokens &&
        special_tokens_set.count(added_token.content) > 0) {
      continue;
    }
    if (added_token.single_word) {
      bool start_space =
          start == 0 || !ends_with_word(sentence.substr(0, start));
      bool stop_space =
          stop == sentence.length() || !starts_with_word(sentence.substr(stop));
      if (!stop_space || !start_space) {
        continue;
      }
    }
    if (added_token.lstrip) {
      int new_start = space_leftmost_at_end(sentence.substr(0, start));
      start = std::max(new_start, start_offset);
    }
    if (added_token.rstrip) {
      stop += space_rightmost_at_start(sentence.substr(stop));
    }
    if (start_offset < start) {
      result.push_back({std::nullopt, {start_offset, start}});
    }
    result.push_back({id, {start, stop}});
    start_offset = stop;
  }
  int total_len = unicode_sentence.length();
  if (start_offset != total_len) {
    result.push_back({std::nullopt, {start_offset, total_len}});
  }
  return result;
}

int AddedVocabulary::add_special_tokens(const std::vector<AddedToken>& tokens,
                                        Model* model, Normalizer* normalizer) {
  return add_tokens(tokens, model, normalizer);
}

PreTokenizedString AddedVocabulary::extract_and_normalize(
    const Normalizer* normalizer, const std::wstring& sequence) {
  PreTokenizedString pre_tokenized =
      PreTokenizedString(NormalizedString(sequence));
  auto matches =
      find_matches(convert_to_string(pre_tokenized.normalized.normalized),
                   split_non_normalized_trie);
  std::vector<Split> non_normalized_splits;
  for (auto match : matches) {
    std::string value =
        convert_to_string(pre_tokenized.normalized.normalized.substr(
            match.second.first, match.second.second - match.second.first));
    if (match.first.has_value()) {
      Split new_split = Split(value, match.second);
      new_split.tokens = {
          Token(match.first.value(), value, {0, value.length()})};
      non_normalized_splits.push_back(new_split);
    } else {
      non_normalized_splits.push_back(Split(value, match.second));
    }
  }
  if (non_normalized_splits.size() > 0) {
    pre_tokenized.splits = non_normalized_splits;
  }

  std::vector<Split> normalized_splits;
  int ending_idx = 0;
  for (const Split& original_split : pre_tokenized.splits) {
    if (original_split.tokens.size() > 0) {
      Split new_split = original_split;
      new_split.offsets = {ending_idx,
                           ending_idx + new_split.normalized.length()};
      normalized_splits.push_back(new_split);
      ending_idx += new_split.normalized.length();
    } else {
      NormalizedString split_normalized =
          NormalizedString(convert_from_string(original_split.normalized));
      if (normalizer != nullptr) {
        split_normalized = normalizer->normalize(split_normalized);
      }
      std::pair<int, int> new_split_offset = {
          ending_idx, ending_idx + original_split.offsets.second -
                          original_split.offsets.first};
      pre_tokenized.normalized.transform_range(new_split_offset,
                                               split_normalized);

      matches = find_matches(convert_to_string(split_normalized.normalized),
                             split_normalized_trie);
      for (auto match : matches) {
        std::string value =
            convert_to_string(split_normalized.normalized.substr(
                match.second.first, match.second.second - match.second.first));
        if (match.first.has_value()) {
          Split new_split =
              Split(value, {match.second.first + new_split_offset.first,
                            match.second.second + new_split_offset.first});
          new_split.tokens = {
              Token(match.first.value(), value, {0, value.length()})};
          normalized_splits.push_back(new_split);
        } else {
          normalized_splits.push_back(
              Split(value, {match.second.first + new_split_offset.first,
                            match.second.second + new_split_offset.first}));
        }
      }
      ending_idx += split_normalized.normalized.length();
    }
  }
  if (normalized_splits.size() > 0) {
    pre_tokenized.splits = normalized_splits;
  }

  return pre_tokenized;
}

std::unique_ptr<AddedVocabulary> with_added_vocabulary(
    simdjson::ondemand::array added_tokens_params) {
  std::vector<AddedToken> added_tokens;
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
  return std::make_unique<AddedVocabulary>(AddedVocabulary(added_tokens));
}
