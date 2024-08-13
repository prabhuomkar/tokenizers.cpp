// Copyright 2024 Omkar Prabhu
#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"
#include "tokenizers/pre_tokenizer.h"

enum MODEL {
  BPE_MODEL,
  UNIGRAM_MODEL,
  WORD_LEVEL_MODEL,
  WORD_PIECE_MODEL,
  UNKNOWN_MODEL
};

MODEL get_model(std::string type);

class Model {
 public:
  std::unordered_map<std::string, int> vocab;
  std::unordered_map<int, std::string> vocab_r;
  virtual ~Model() = default;
  virtual PreTokenizedString tokenize(
      PreTokenizedString pre_tokenized) const = 0;
  explicit Model(const std::unordered_map<std::string, int>& vocab);
  int get_vocab_size() const;
  std::optional<int> token_to_id(std::string token);
  std::optional<std::string> id_to_token(int id);
};

std::unique_ptr<Model> with_model(simdjson::ondemand::object model_params);

class WordPiece : public Model {
 public:
  std::string unk_token;
  int max_input_chars_per_word;
  std::string continuing_subword_prefix;
  PreTokenizedString tokenize(PreTokenizedString pre_tokenized) const override;
  explicit WordPiece(const std::unordered_map<std::string, int>& vocab,
                     const std::string& unk_token = "[UNK]",
                     int max_input_chars_per_word = 100,
                     const std::string& continuing_subword_prefix = "##");
};

class Symbol {
 public:
  int c;
  int prev;
  int next;
  int len;
  Symbol(int c, int prev, int next, int len);
  void merge_with(const Symbol* other, int new_c);
};

class Merge {
 public:
  int pos;
  int rank;
  int new_id;
  Merge(int pos, int rank, int new_id);
  bool operator<(const Merge& other) const { return rank < other.rank; }
};

struct PairHash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2>& p) const {
    auto h1 = std::hash<T1>{}(p.first);
    auto h2 = std::hash<T2>{}(p.second);
    return h1 ^ (h2 << 1);
  }
};

class Word {
 public:
  std::vector<Symbol> symbols;
  Word() = default;
  explicit Word(const std::vector<Symbol>& symbols);
  void add(int c, int len);
  void merge_all(
      std::unordered_map<std::pair<int, int>, std::pair<int, int>, PairHash>
          merges,
      float dropout);
};

class BPE : public Model {
 public:
  std::unordered_map<std::pair<int, int>, std::pair<int, int>, PairHash> merges;
  float dropout;
  std::string unk_token;
  std::string continuing_subword_prefix;
  std::string end_of_word_suffix;
  bool fuse_unk;
  bool byte_fallback;
  bool ignore_merges;
  PreTokenizedString tokenize(PreTokenizedString pre_tokenized) const override;
  explicit BPE(const std::unordered_map<std::string, int>& vocab,
               const std::vector<std::string>& merges_list, float dropout,
               const std::string& unk_token,
               const std::string& continuing_subword_prefix,
               const std::string& end_of_word_suffix, bool fuse_unk,
               bool byte_fallback, bool ignore_merges);

 private:
  mutable std::unordered_map<std::string, Word> cache;
  Word merge_word(std::string sequence) const;
  std::vector<Token> word_to_tokens(const Word& word) const;
  std::vector<Token> tokenize_with_cache(const std::string& sequence) const;
};
