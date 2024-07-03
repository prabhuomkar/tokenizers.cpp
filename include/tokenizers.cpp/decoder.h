// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

enum DECODER {
  BPE_DECODER,
  BYTE_LEVEL_DECODER,
  CTC_DECODER,
  METASPACE_DECODER,
  WORD_PIECE_DECODER,
  UNKNOWN_DECODER
};

DECODER get_decoder(std::string_view type);

class Decoder {
 public:
  Decoder();
};

class WordPieceDecoder : public Decoder {
 public:
  WordPieceDecoder();
};
