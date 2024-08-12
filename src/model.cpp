// Copyright 2024 Omkar Prabhu
#include "tokenizers/model.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"
#include "tokenizers/utils.h"

MODEL get_model(std::string type) {
  static const std::unordered_map<std::string, MODEL> types = {
      {"BPE", BPE_MODEL},
      {"Unigram", UNIGRAM_MODEL},
      {"WordLevel", WORD_LEVEL_MODEL},
      {"WordPiece", WORD_PIECE_MODEL}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_MODEL;
}

std::unique_ptr<Model> with_model(simdjson::ondemand::object model_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(model_params["type"].get_string()));
  val = model_params["vocab"].value();
  std::unordered_map<std::string, int> vocab =
      val.type() == simdjson::ondemand::json_type::null
          ? std::unordered_map<std::string, int>()
          : get_map_ints_from_json(val.get_object());
  if (get_model(type) == WORD_PIECE_MODEL) {
    val = model_params["unk_token"].value();
    std::string unk_token =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["max_input_chars_per_word"].value();
    int max_input_chars_per_word =
        val.type() == simdjson::ondemand::json_type::null
            ? 0
            : static_cast<int>(val.get_int64());
    val = model_params["continuing_subword_prefix"].value();
    std::string continuing_subword_prefix =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    return std::make_unique<WordPiece>(WordPiece(
        vocab, unk_token, max_input_chars_per_word, continuing_subword_prefix));
  } else if (get_model(type) == BPE_MODEL) {
    val = model_params["merges"].value();
    std::vector<std::string> merges;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::array merges_array = val.get_array();
      for (auto element : merges_array) {
        merges.push_back(
            std::string(static_cast<std::string_view>(element.get_string())));
      }
    }
    val = model_params["dropout"].value();
    float dropout = val.type() == simdjson::ondemand::json_type::null
                        ? 0.0
                        : static_cast<float>(val.get_double());
    val = model_params["unk_token"].value();
    std::string unk_token =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["continuing_subword_prefix"].value();
    std::string continuing_subword_prefix =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["end_of_word_suffix"].value();
    std::string end_of_word_suffix =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["fuse_unk"].value();
    bool fuse_unk = val.type() == simdjson::ondemand::json_type::null
                        ? false
                        : static_cast<bool>(val.get_bool());
    val = model_params["byte_fallback"].value();
    bool byte_fallback = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());
    val = model_params["ignore_merges"].value();
    bool ignore_merges = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());
    return std::make_unique<BPE>(
        BPE(vocab, merges, dropout, unk_token, continuing_subword_prefix,
            end_of_word_suffix, fuse_unk, byte_fallback, ignore_merges));
  }
  return nullptr;
}

Model::Model(const std::unordered_map<std::string, int>& vocab) : vocab(vocab) {
  for (auto pair : vocab) {
    vocab_r[pair.second] = pair.first;
  }
}

int Model::get_vocab_size() const { return vocab.size(); }

std::optional<int> Model::token_to_id(std::string token) {
  auto it = vocab.find(token);
  if (it != vocab.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::optional<std::string> Model::id_to_token(int id) {
  auto it = vocab_r.find(id);
  if (it != vocab_r.end()) {
    return it->second;
  }
  return std::nullopt;
}

WordPiece::WordPiece(const std::unordered_map<std::string, int>& vocab,
                     const std::string& unk_token, int max_input_chars_per_word,
                     const std::string& continuing_subword_prefix)
    : Model(vocab),
      unk_token(unk_token),
      max_input_chars_per_word(max_input_chars_per_word),
      continuing_subword_prefix(continuing_subword_prefix) {}

PreTokenizedString WordPiece::tokenize(PreTokenizedString pre_tokenized) const {
  for (auto& split : pre_tokenized.splits) {
    icu::UnicodeString unicode_sequence =
        icu::UnicodeString::fromUTF8(split.normalized);
    int char_len = unicode_sequence.length();
    if (char_len > max_input_chars_per_word) {
      auto it = vocab.find(unk_token);
      if (it == vocab.end()) {
        split.tokens = {Token(0, unk_token, {0, char_len})};
        continue;
      }
      split.tokens = {Token(it->second, unk_token, {0, char_len})};
      continue;
    }
    bool is_bad = false;
    int start = 0;
    std::vector<Token> sub_tokens;
    while (start < unicode_sequence.length()) {
      int end = unicode_sequence.length();
      std::optional<Token> cur_sequence_token = std::nullopt;
      while (start < end) {
        std::string sub_sequence;
        icu::UnicodeString unicode_sub_sequence =
            unicode_sequence.tempSubString(start, end - start);
        unicode_sub_sequence.toUTF8String(sub_sequence);
        if (start > 0) {
          sub_sequence = continuing_subword_prefix + sub_sequence;
        }
        if (vocab.count(sub_sequence) != 0) {
          auto it = vocab.find(sub_sequence);
          if (it != vocab.end()) {
            cur_sequence_token = Token(it->second, sub_sequence, {start, end});
          }
          break;
        }
        end -= 1;
      }
      if (!cur_sequence_token.has_value()) {
        is_bad = true;
        break;
      }
      sub_tokens.push_back(cur_sequence_token.value());
      start = end;
    }
    if (is_bad) {
      auto it = vocab.find(unk_token);
      if (it == vocab.end()) {
        split.tokens = {Token(0, unk_token, {0, char_len})};
        continue;
      }
      split.tokens = {Token(it->second, unk_token, {0, char_len})};
      continue;
    }
    split.tokens = sub_tokens;
  }
  return pre_tokenized;
}

Symbol::Symbol(int c, int prev, int next, int len)
    : c(c), prev(prev), next(next), len(len) {}

void Symbol::merge_with(const Symbol* other, int new_id) { c = new_id; }

Merge::Merge(int pos, int rank, int new_id)
    : pos(pos), rank(rank), new_id(new_id) {}

Word::Word(const std::vector<Symbol>& symbols) : symbols(symbols) {}

void Word::add(int c, int len) {
  int prev = -1, next = -1;
  int cur_len = symbols.size();
  if (!symbols.empty()) {
    Symbol& last = symbols.back();
    last.next = cur_len;
    prev = cur_len - 1;
  }
  symbols.emplace_back(Symbol(c, prev, next, len));
}

void Word::merge_all(
    std::unordered_map<std::pair<int, int>, std::pair<int, int>, PairHash>
        merges,
    float dropout) {
  std::priority_queue<Merge> queue;
  std::vector<Merge> skip;
  for (int i = 0; i < symbols.size() - 1; i++) {
    std::pair<int, int> pair = {symbols[i].c, symbols[i + 1].c};
    auto it = merges.find(pair);
    if (it != merges.end()) {
      queue.emplace(i, it->second.first, it->second.second);
    }
  }
  std::default_random_engine rng;
  std::uniform_real_distribution<float> dist(0.0, 1.0);
  while (!queue.empty()) {
    Merge top = queue.top();
    queue.pop();
    if (dropout != 0.0f && dist(rng) < dropout) {
      skip.push_back(top);
      continue;
    }
    for (const auto& item : skip) {
      queue.push(item);
    }
    skip.clear();
    if (symbols[top.pos].len == 0) {
      continue;
    }
    if (symbols[top.pos].next == -1) {
      continue;
    }
    int next_pos = symbols[top.pos].next;
    if (next_pos >= symbols.size()) {
      continue;
    }
    const Symbol& right = symbols[next_pos];
    std::pair<int, int> target_new_pair = {symbols[top.pos].c, right.c};
    auto target_it = merges.find(target_new_pair);
    if (target_it == merges.end() || target_it->second.second != top.new_id) {
      continue;
    }
    symbols[top.pos].merge_with(&right, top.new_id);
    symbols[next_pos].len = 0;
    if (right.next >= 0 && right.next < symbols.size()) {
      symbols[right.next].prev = top.pos;
    }
    const Symbol& current = symbols[top.pos];
    if (current.prev >= 0) {
      int prev = current.prev;
      if (prev < symbols.size()) {
        const Symbol& prev_symbol = symbols[prev];
        std::pair<int, int> new_pair = {prev_symbol.c, current.c};
        auto new_it = merges.find(new_pair);
        if (new_it != merges.end()) {
          queue.emplace(prev, new_it->second.first, new_it->second.second);
        }
      }
    }
    int next = current.next;
    if (next < symbols.size()) {
      const Symbol& next_symbol = symbols[next];
      std::pair<int, int> new_pair = {current.c, next_symbol.c};
      auto new_it = merges.find(new_pair);
      if (new_it != merges.end()) {
        queue.emplace(top.pos, new_it->second.first, new_it->second.second);
      }
    }
  }
  symbols.erase(std::remove_if(symbols.begin(), symbols.end(),
                               [](const Symbol& s) { return s.len == 0; }),
                symbols.end());
}

BPE::BPE(const std::unordered_map<std::string, int>& vocab,
         const std::vector<std::string>& merges_list, float dropout,
         const std::string& unk_token,
         const std::string& continuing_subword_prefix,
         const std::string& end_of_word_suffix, bool fuse_unk,
         bool byte_fallback, bool ignore_merges)
    : Model(vocab),
      merges(),
      dropout(dropout),
      unk_token(unk_token),
      continuing_subword_prefix(continuing_subword_prefix),
      end_of_word_suffix(end_of_word_suffix),
      fuse_unk(fuse_unk),
      byte_fallback(byte_fallback),
      ignore_merges(ignore_merges),
      cache({}) {
  int prefix_len = continuing_subword_prefix.length();
  for (int i = 0; i < merges.size(); ++i) {
    std::istringstream iss(merges_list[i]);
    std::string part1, part2;
    if (iss >> part1 >> part2) {
      auto it1 = vocab.find(part1);
      auto it2 = vocab.find(part2);
      if (it1 != vocab.end() && it2 != vocab.end()) {
        int a_id = it1->second;
        int b_id = it2->second;
        std::string new_token =
            part1 + (prefix_len != 0 ? part2.substr(prefix_len) : part2);
        auto it_new = vocab.find(new_token);
        if (it_new != vocab.end()) {
          int new_id = it_new->second;
          merges.insert({{a_id, b_id}, {i, new_id}});
        }
      }
    }
  }
}

Word BPE::merge_word(std::string sequence) const {
  int length = sequence.size();
  Word word;
  std::optional<std::pair<int, int>> unk;
  for (int i = 0; i < length; i++) {
    int end = (i + 1 < length) ? i + 1 : length;
    bool is_first = (i == 0);
    bool is_last = (end == length);

    std::string sub_sequence = sequence.substr(i, end - i);
    int sub_len = sub_sequence.size();

    if (!is_first && continuing_subword_prefix.size() != 0) {
      sub_sequence = (continuing_subword_prefix + sub_sequence);
    }

    if (is_last && end_of_word_suffix.size() != 0) {
      sub_sequence += end_of_word_suffix;
    }

    auto it = vocab.find(sub_sequence);
    if (it != vocab.end()) {
      if (unk.has_value()) {
        word.add(unk->first, unk->second);
        unk.reset();
      }
      word.add(it->second, sub_len);
    } else {
      if (byte_fallback) {
        std::vector<std::optional<int>> tokens;
        for (auto b : sub_sequence) {
          std::ostringstream oss;
          oss << "<" << std::hex << std::setw(4) << std::setfill('0') << b
              << ">";
          auto code = oss.str();
          auto token_it = vocab.find(code);
          tokens.push_back((token_it != vocab.end())
                               ? std::optional<int>(token_it->second)
                               : std::nullopt);
        }
        bool added = false;
        for (auto& token : tokens) {
          if (token.has_value()) {
            word.add(token.value(), 1);
            added = true;
          }
        }
        if (added) continue;
      }
      if (unk_token.size() != 0) {
        if (unk.has_value() && fuse_unk) {
          unk = {unk->first, unk->second + sub_len};
        } else {
          if (unk.has_value()) {
            word.add(unk->first, unk->second);
          }
          auto unk_it = vocab.find(unk_token);
          if (unk_it != vocab.end()) {
            unk = {unk_it->second, sub_len};
          }
        }
      }
    }
  }
  if (unk.has_value()) {
    word.add(unk->first, unk->second);
  }
  word.merge_all(merges, dropout);
  return word;
}

std::vector<Token> BPE::word_to_tokens(const Word& word) const {
  std::vector<Token> result;
  int pos = 0;
  for (auto symbol : word.symbols) {
    int new_pos = pos + symbol.len;
    result.push_back(Token(symbol.c, vocab_r.at(symbol.c), {pos, new_pos}));
    pos = new_pos;
  }
  return result;
}

std::vector<Token> BPE::tokenize_with_cache(const std::string& sequence) const {
  if (ignore_merges) {
    auto it = vocab.find(sequence);
    if (it != vocab.end()) {
      return {Token(it->second, sequence, {0, 0})};
    }
  }
  auto it = cache.find(sequence);
  if (it != cache.end()) {
    return word_to_tokens(it->second);
  }
  auto word = merge_word(sequence);
  auto result = word_to_tokens(word);
  cache.insert({sequence, word});
  return result;
}

PreTokenizedString BPE::tokenize(PreTokenizedString pre_tokenized) const {
  for (auto& split : pre_tokenized.splits) {
    std::string sequence = split.normalized;
    if (dropout == 0.0f) {
      split.tokens = tokenize_with_cache(sequence);
    } else {
      auto word = merge_word(sequence);
      split.tokens = word_to_tokens(word);
    }
  }
  return pre_tokenized;
}
