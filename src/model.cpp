// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/model.h"

#include <iostream>
#include <string>
#include <unordered_map>

MODEL get_model(std::string_view type) {
  static const std::unordered_map<std::string_view, MODEL> types = {
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

Model::Model() {}

WordPiece::WordPiece() {
  std::cout << "Initialized Model: WordPiece" << std::endl;
}
