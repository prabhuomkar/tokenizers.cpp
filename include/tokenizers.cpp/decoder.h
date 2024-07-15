// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

#include "simdjson.h"

enum DECODER {
  BPE_DECODER,
  BYTE_LEVEL_DECODER,
  CTC_DECODER,
  METASPACE_DECODER,
  WORD_PIECE_DECODER,
  UNKNOWN_DECODER
};

DECODER get_decoder(std::string type);

class Decoder {
 public:
  Decoder();
};

Decoder with_decoder(simdjson::ondemand::object decoder_params);

class WordPieceDecoder : public Decoder {
 public:
  explicit WordPieceDecoder(std::string prefix = "##", bool cleanup = true);
};
