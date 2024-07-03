// Copyright 2024 Omkar Prabhu
#pragma once

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

MODEL get_model(std::string_view type);

class ModelConfig {
 public:
  std::string_view type;
  std::unordered_map<std::string_view, int> vocab;
  std::string_view unk_token;
  int max_input_chars_per_word;
  explicit ModelConfig(simdjson::ondemand::object model_params);
};

class Model {
 public:
  Model();
};

class WordPiece : public Model {
 public:
  WordPiece(std::unordered_map<std::string_view, int> vocab,
            std::string_view unk_token, int max_input_chars_per_word);
};
