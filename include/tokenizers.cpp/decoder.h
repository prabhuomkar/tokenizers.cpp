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

DECODER get_decoder(std::string_view type);

class DecoderConfig {
 public:
  std::string_view type;
  std::string_view prefix;
  bool cleanup;
  explicit DecoderConfig(simdjson::ondemand::object decoder_params);
};

class Decoder {
 public:
  Decoder();
};

class WordPieceDecoder : public Decoder {
 public:
  explicit WordPieceDecoder(std::string_view prefix = "##",
                            bool cleanup = true);
};
