// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/tokenizer.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "simdjson.h"
#include "tokenizers.cpp/added_vocabulary.h"
#include "tokenizers.cpp/common.h"
#include "tokenizers.cpp/decoder.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"
#include "tokenizers.cpp/post_processor.h"
#include "tokenizers.cpp/pre_tokenizer.h"
#include "tokenizers.cpp/utils.h"

Tokenizer::Tokenizer(std::string path) {
  // load json from file tokenizer.json
  simdjson::ondemand::parser parser;
  simdjson::padded_string tokenizer_json_str =
      simdjson::padded_string::load(path + "/tokenizer.json");
  simdjson::ondemand::document tokenizer_json =
      parser.iterate(tokenizer_json_str);

  // set truncation, padding, added_tokens
  if (!tokenizer_json["added_tokens"].is_null()) {
    simdjson::ondemand::array added_tokens_params =
        tokenizer_json["added_tokens"].get_array();
    added_vocabulary =
        with_added_vocabulary(AddedVocabularyConfig(added_tokens_params));
  }

  // initialize the components of the tokenizer e.g. normalizer, pre_tokenizer
  // model, decoder, post_processor
  if (!tokenizer_json["normalizer"].is_null()) {
    simdjson::ondemand::object normalizer_params =
        tokenizer_json["normalizer"].value();
    normalizer = with_normalizer(normalizer_params);
  }
  if (!tokenizer_json["pre_tokenizer"].is_null()) {
    simdjson::ondemand::object pre_tokenizer_params =
        tokenizer_json["pre_tokenizer"].value();
    pre_tokenizer = with_pre_tokenizer(pre_tokenizer_params);
  }
  if (!tokenizer_json["model"].is_null()) {
    simdjson::ondemand::object model_params = tokenizer_json["model"].value();
    model = with_model(model_params);
  }
  if (!tokenizer_json["decoder"].is_null()) {
    simdjson::ondemand::object decoder_params =
        tokenizer_json["decoder"].value();
    decoder = with_decoder(decoder_params);
  }
  if (!tokenizer_json["post_processor"].is_null()) {
    simdjson::ondemand::object post_processor_params =
        tokenizer_json["post_processor"].value();
    post_processor = with_post_processor(post_processor_params);
  }

  add_tokens(added_vocabulary.added_tokens);
}

Encoding Tokenizer::encode(std::string sequence, bool is_pretokenized,
                           bool add_special_tokens) {
  // normalize
  // pretokenize
  // tokenize using model
  return Encoding();
}

std::string Tokenizer::decode(std::vector<int> ids, bool skip_special_tokens) {
  return "";
}

int Tokenizer::add_tokens(std::vector<AddedToken> tokens) {
  return added_vocabulary.add_tokens(tokens, model, std::move(normalizer));
}

int Tokenizer::add_special_tokens(std::vector<AddedToken> tokens) {
  return added_vocabulary.add_special_tokens(tokens, model,
                                             std::move(normalizer));
}

AddedVocabulary Tokenizer::with_added_vocabulary(
    AddedVocabularyConfig added_vocabulary_config) {
  return AddedVocabulary(added_vocabulary_config.added_tokens);
}
