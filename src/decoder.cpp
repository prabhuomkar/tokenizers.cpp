// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/decoder.h"

#include <iostream>
#include <string>
#include <unordered_map>

DECODER get_decoder(std::string_view type) {
  static const std::unordered_map<std::string_view, DECODER> types = {
      {"BPEDecoder", BPE_DECODER},
      {"ByteLevel", BYTE_LEVEL_DECODER},
      {"CTC", CTC_DECODER},
      {"Metaspace", METASPACE_DECODER},
      {"WordPiece", WORD_PIECE_DECODER}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_DECODER;
}

Decoder::Decoder() {}

WordPieceDecoder::WordPieceDecoder() {
  std::cout << "Initialized Decoder: WordPiece" << std::endl;
}
