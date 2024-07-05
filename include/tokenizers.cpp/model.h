// Copyright 2024 Omkar Prabhu
#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "simdjson.h"

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
  explicit ModelConfig(simdjson::ondemand::object model_params);
};

class Model {
 public:
  Model();
  Model(std::unordered_map<std::string, int> vocab, std::string unk_token,
        int max_input_chars_per_word);
  int get_vocab_size();
  std::optional<int> token_to_id(std::string token);

 private:
  std::unordered_map<std::string, int> vocab;
  std::string unk_token;
  int max_input_chars_per_word;
};

class WordPiece : public Model {
 public:
  WordPiece(std::unordered_map<std::string, int> vocab, std::string unk_token,
            int max_input_chars_per_word);
};
