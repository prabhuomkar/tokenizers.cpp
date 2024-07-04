// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/model.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include "simdjson.h"
#include "tokenizers.cpp/utils.h"

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

ModelConfig::ModelConfig(simdjson::ondemand::object model_params) {
  simdjson::ondemand::value val;
  type = model_params["type"].get_string();

  val = model_params["vocab"].value();
  vocab = val.type() == simdjson::ondemand::json_type::null
              ? std::unordered_map<std::string_view, int>()
              : get_map_ints_from_json(val.get_object());

  val = model_params["unk_token"].value();
  unk_token = val.type() == simdjson::ondemand::json_type::null
                  ? ""
                  : static_cast<std::string_view>(val.get_string());

  val = model_params["max_input_chars_per_word"].value();
  max_input_chars_per_word = val.type() == simdjson::ondemand::json_type::null
                                 ? 0
                                 : static_cast<int>(val.get_int64());
}

Model::Model() {}

WordPiece::WordPiece(std::unordered_map<std::string_view, int> vocab,
                     std::string_view unk_token, int max_input_chars_per_word) {
  std::cout << "Initialized Model: WordPiece" << std::endl;
  std::cout << "params: " << unk_token << " " << max_input_chars_per_word
            << std::endl;
}
