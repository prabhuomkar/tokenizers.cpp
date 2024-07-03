// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/decoder.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include "simdjson.h"

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

DecoderConfig::DecoderConfig(simdjson::ondemand::object decoder_params) {
  simdjson::ondemand::value val;
  type = decoder_params["type"].get_string();

  val = decoder_params["prefix"].value();
  prefix = val.type() == simdjson::ondemand::json_type::null
               ? ""
               : static_cast<std::string_view>(val.get_string());

  val = decoder_params["cleanup"].value();
  cleanup = val.type() == simdjson::ondemand::json_type::null
                ? false
                : static_cast<bool>(val.get_bool());
}

Decoder::Decoder() {}

WordPieceDecoder::WordPieceDecoder(std::string_view prefix, bool cleanup) {
  std::cout << "Initialized Decoder: WordPiece" << std::endl;
  std::cout << "params: " << prefix << " " << cleanup << std::endl;
}
