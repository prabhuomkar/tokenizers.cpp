// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/decoder.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

std::unique_ptr<Decoder> with_decoder(
    simdjson::ondemand::object decoder_params) {
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
    return std::make_unique<WordPieceDecoder>(
        WordPieceDecoder(prefix, cleanup));
  }
  return nullptr;
}

WordPieceDecoder::WordPieceDecoder(std::string prefix, bool cleanup)
    : prefix(prefix), cleanup(cleanup) {}

std::string replace(std::string input, std::string from, std::string to) {
  int start = 0;
  while ((start = input.find(from, start)) != std::string::npos) {
    input.replace(start, from.length(), to);
    start += to.length();
  }
  return input;
}

std::string cleanup_token(std::string dirty_input) {
  dirty_input = replace(dirty_input, " .", ".");
  dirty_input = replace(dirty_input, " ?", "?");
  dirty_input = replace(dirty_input, " !", "!");
  dirty_input = replace(dirty_input, " ,", ",");
  dirty_input = replace(dirty_input, " ' ", "'");
  dirty_input = replace(dirty_input, " n't", "n't");
  dirty_input = replace(dirty_input, " 'm", "'m");
  dirty_input = replace(dirty_input, " do not", " don't");
  dirty_input = replace(dirty_input, " 's", "'s");
  dirty_input = replace(dirty_input, " 've", "'ve");
  dirty_input = replace(dirty_input, " 're", "'re");
  return dirty_input;
}

std::vector<std::string> WordPieceDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  std::vector<std::string> result;
  for (int i = 0; i < tokens.size(); i++) {
    std::string token = tokens[i];
    if (i != 0) {
      if (token.find(prefix) == 0) {
        token.replace(0, prefix.length(), "");
      } else {
        token = " " + token;
      }
    }
    if (cleanup) {
      token = cleanup_token(token);
    }
    result.push_back(token);
  }
  return result;
}
