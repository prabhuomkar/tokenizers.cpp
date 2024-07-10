// Copyright 2024 Omkar Prabhu
#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.h"
#include "tokenizers.cpp/common.h"

enum MODEL {
  BPE_MODEL,
  UNIGRAM_MODEL,
  WORD_LEVEL_MODEL,
  WORD_PIECE_MODEL,
  UNKNOWN_MODEL
};

MODEL get_model(std::string type);

class ModelConfig {
 public:
  std::string type;
  std::unordered_map<std::string, int> vocab;
  std::string unk_token;
  int max_input_chars_per_word;
  std::string continuing_subword_prefix;
  explicit ModelConfig(simdjson::ondemand::object model_params);
};

class Model {
 public:
  Model();
  explicit Model(std::unordered_map<std::string, int> vocab);
  int get_vocab_size();
  std::optional<int> token_to_id(std::string token);

 private:
  std::unordered_map<std::string, int> vocab;
};

class WordPiece : public Model {
 public:
  std::string type;
  std::unordered_map<std::string, int> vocab;
  std::string unk_token;
  int max_input_chars_per_word;
  std::string continuing_subword_prefix;
  std::vector<Token> tokenize(std::string sequence);
  WordPiece(std::unordered_map<std::string, int> vocab, std::string unk_token,
            int max_input_chars_per_word,
            std::string continuing_subword_prefix);
};
