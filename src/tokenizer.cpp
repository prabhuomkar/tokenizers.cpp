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

Encoding Tokenizer::encode(std::wstring sequence) {
  std::vector<Split> splits;
  if (normalizer != nullptr) {
    sequence = normalizer->normalize(sequence);
  }
  if (pre_tokenizer != nullptr) {
    splits = pre_tokenizer->pre_tokenize(sequence);
  }
  return do_tokenize(splits, std::nullopt, 0);
}

std::string Tokenizer::decode(std::vector<int> ids, bool skip_special_tokens) {
  return "";
}

int Tokenizer::add_tokens(std::vector<AddedToken> tokens) {
  return added_vocabulary.add_tokens(tokens, model.get(), normalizer.get());
}

int Tokenizer::add_special_tokens(std::vector<AddedToken> tokens) {
  return added_vocabulary.add_special_tokens(tokens, model.get(),
                                             normalizer.get());
}

Encoding into_encoding(std::vector<Split> splits, std::optional<int> word_idx,
                       int type_id) {
  Encoding encoding;
  for (int idx = 0; idx < splits.size(); idx++) {
    Split split = splits[idx];
    for (Token token : split.tokens) {
      encoding.ids.push_back(token.id);
      encoding.tokens.push_back(token.value);
      encoding.offsets.push_back({token.offsets.first + split.offsets.first,
                                  token.offsets.first + split.offsets.second});
      encoding.words.push_back(word_idx.has_value() ? word_idx.value() : idx);
      encoding.type_ids.push_back(type_id);
    }
  }
  return encoding;
}

Encoding Tokenizer::do_tokenize(std::vector<Split> splits,
                                std::optional<int> word_idx, int type_id) {
  for (Split &split : splits) {
    std::vector<Token> split_tokens = model->tokenize(split.normalized);
    split.tokens = split_tokens;
  }
  return into_encoding(splits, word_idx, type_id);
}

AddedVocabulary Tokenizer::with_added_vocabulary(
    AddedVocabularyConfig added_vocabulary_config) {
  return AddedVocabulary(added_vocabulary_config.added_tokens);
}
