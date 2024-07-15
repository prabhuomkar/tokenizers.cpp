// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/decoder.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include "simdjson.h"

DECODER get_decoder(std::string type) {
  static const std::unordered_map<std::string, DECODER> types = {
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

Decoder with_decoder(simdjson::ondemand::object decoder_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(decoder_params["type"].get_string()));
  if (get_decoder(type) == WORD_PIECE_DECODER) {
    val = decoder_params["prefix"].value();
    std::string prefix =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(val.get_string()));
    val = decoder_params["cleanup"].value();
    bool cleanup = val.type() == simdjson::ondemand::json_type::null
                       ? false
                       : static_cast<bool>(val.get_bool());
    return WordPieceDecoder(prefix, cleanup);
  }
  return Decoder();
}

WordPieceDecoder::WordPieceDecoder(std::string prefix, bool cleanup) {
  std::cout << "Initialized Decoder: WordPiece" << std::endl;
  std::cout << "params: " << prefix << " " << cleanup << std::endl;
}
