// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

enum MODEL {
  BPE_MODEL,
  UNIGRAM_MODEL,
  WORD_LEVEL_MODEL,
  WORD_PIECE_MODEL,
  UNKNOWN_MODEL
};

MODEL get_model(std::string_view type);

class Model {
 public:
  Model();
};

class WordPiece : public Model {
 public:
  WordPiece();
};
