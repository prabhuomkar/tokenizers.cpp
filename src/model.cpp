// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/model.h"

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "simdjson.h"
#include "tokenizers.cpp/common.h"
#include "tokenizers.cpp/utils.h"

MODEL get_model(std::string type) {
  static const std::unordered_map<std::string, MODEL> types = {
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
  type = std::string(
      static_cast<std::string_view>(model_params["type"].get_string()));

  val = model_params["vocab"].value();
  vocab = val.type() == simdjson::ondemand::json_type::null
              ? std::unordered_map<std::string, int>()
              : get_map_ints_from_json(val.get_object());

  val = model_params["unk_token"].value();
  unk_token =
      val.type() == simdjson::ondemand::json_type::null
          ? ""
          : std::string(static_cast<std::string_view>(val.get_string()));

  val = model_params["max_input_chars_per_word"].value();
  max_input_chars_per_word = val.type() == simdjson::ondemand::json_type::null
                                 ? 0
                                 : static_cast<int>(val.get_int64());

  val = model_params["continuing_subword_prefix"].value();
  continuing_subword_prefix =
      val.type() == simdjson::ondemand::json_type::null
          ? ""
          : std::string(static_cast<std::string_view>(val.get_string()));
}

Model::Model() {}

Model::Model(std::unordered_map<std::string, int> vocab) : vocab(vocab) {}

int Model::get_vocab_size() { return vocab.size(); }

std::optional<int> Model::token_to_id(std::string token) {
  auto it = vocab.find(token);
  if (it != vocab.end()) {
    return it->second;
  }
  return std::nullopt;
}

WordPiece::WordPiece(std::unordered_map<std::string, int> vocab,
                     std::string unk_token, int max_input_chars_per_word,
                     std::string continuing_subword_prefix)
    : Model(vocab),
      vocab(vocab),
      unk_token(unk_token),
      max_input_chars_per_word(max_input_chars_per_word),
      continuing_subword_prefix(continuing_subword_prefix) {
  std::cout << "Initialized Model: WordPiece" << std::endl;
  std::cout << "params: " << vocab.size() << " " << unk_token << " "
            << max_input_chars_per_word << std::endl;
}

std::vector<Token> WordPiece::tokenize(std::string sequence) {
  int char_len = sequence.length();
  if (char_len > max_input_chars_per_word) {
    return {Token(vocab[unk_token], unk_token, {0, char_len})};
  }
  bool is_bad = false;
  int start = 0;
  std::vector<Token> sub_tokens;
  while (start < sequence.length()) {
    int end = sequence.length();
    std::optional<Token> cur_sequence_token = std::nullopt;
    while (start < end) {
      std::string sub_sequence = sequence.substr(start, end);
      if (start > 0) {
        sub_sequence = continuing_subword_prefix + sub_sequence;
      }
      if (vocab.count(sub_sequence) != 0) {
        cur_sequence_token =
            Token(vocab[sub_sequence], sub_sequence, {start, end});
        break;
      }
      end -= sub_sequence.length() - 1;
    }
    if (!cur_sequence_token.has_value()) {
      is_bad = true;
      break;
    }
    sub_tokens.push_back(Token());
    start = end;
  }
  if (is_bad) {
    return {Token(vocab[unk_token], unk_token, {0, char_len})};
  }
  return sub_tokens;
}
