// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/tokenizer.h"

#include <string>
#include <vector>

#include "simdjson.h"
#include "tokenizers.cpp/decoder.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"
#include "tokenizers.cpp/post_processor.h"
#include "tokenizers.cpp/pre_tokenizer.h"

Tokenizer::Tokenizer(std::string path) {
  // load json from file tokenizer.json
  simdjson::ondemand::parser parser;
  simdjson::padded_string tokenizer_json_str =
      simdjson::padded_string::load(path + "/tokenizer.json");
  simdjson::ondemand::document tokenizer_json =
      parser.iterate(tokenizer_json_str);

  // set truncation, padding, added_tokens

  // initialize the components of the tokenizer e.g. normalizer, pre_tokenizer
  // model, post_processor, decoder
  if (!tokenizer_json["normalizer"].is_null()) {
    simdjson::ondemand::object normalizer_params =
        tokenizer_json["normalizer"].value();
    normalizer = with_normalizer(
        normalizer_params["type"],
        normalizer_params["clean_text"].is_null()
            ? false
            : static_cast<bool>(normalizer_params["clean_text"].get_bool()),
        normalizer_params["handle_chinese_chars"].is_null()
            ? false
            : static_cast<bool>(
                  normalizer_params["handle_chinese_chars"].get_bool()),
        normalizer_params["strip_accents"].is_null()
            ? false
            : static_cast<bool>(normalizer_params["strip_accents"].get_bool()),
        normalizer_params["lowercase"].is_null()
            ? false
            : static_cast<bool>(normalizer_params["lowercase"].get_bool()));
  }
  if (!tokenizer_json["pre_tokenizer"].is_null()) {
    simdjson::ondemand::object pre_tokenizer_params =
        tokenizer_json["pre_tokenizer"].value();
    pre_tokenizer =
        with_pre_tokenizer(pre_tokenizer_params["type"].get_string());
  }
  if (!tokenizer_json["model"].is_null()) {
    simdjson::ondemand::object model_params = tokenizer_json["model"].value();
    model = with_model(model_params["type"].get_string());
  }
  if (!tokenizer_json["decoder"].is_null()) {
    simdjson::ondemand::object decoder_params =
        tokenizer_json["decoder"].value();
    decoder = with_decoder(decoder_params["type"].get_string());
  }
  if (!tokenizer_json["post_processor"].is_null()) {
    simdjson::ondemand::object post_processor_params =
        tokenizer_json["post_processor"].value();
    post_processor =
        with_post_processor(post_processor_params["type"].get_string());
  }
}

Encoding Tokenizer::encode(std::string sequence, bool is_pretokenized,
                           bool add_special_tokens) {
  return Encoding();
}

std::string Tokenizer::decode(std::vector<int> ids, bool skip_special_tokens) {
  return "";
}

Normalizer Tokenizer::with_normalizer(std::string_view type, bool clean_text,
                                      bool handle_chinese_chars,
                                      bool strip_accents, bool lowercase) {
  switch (get_normalizer(type)) {
    case BERT_NORMALIZER:
      return BertNormalizer(clean_text, handle_chinese_chars, strip_accents,
                            lowercase);
    default:
      break;
  }

  return Normalizer();
}

PreTokenizer Tokenizer::with_pre_tokenizer(std::string_view type) {
  switch (get_pre_tokenizer(type)) {
    case BERT_PRE_TOKENIZER:
      return BertPreTokenizer();
    default:
      break;
  }

  return PreTokenizer();
}

Model Tokenizer::with_model(std::string_view type) {
  switch (get_model(type)) {
    case WORD_PIECE_MODEL:
      return WordPiece();
    default:
      break;
  }

  return Model();
}

Decoder Tokenizer::with_decoder(std::string_view type) {
  switch (get_decoder(type)) {
    case WORD_PIECE_DECODER:
      return WordPieceDecoder();
    default:
      break;
  }

  return Decoder();
}

PostProcessor Tokenizer::with_post_processor(std::string_view type) {
  switch (get_post_processor(type)) {
    case TEMPLATE_PROCESSING:
      return TemplateProcessing();
    default:
      break;
  }

  return PostProcessor();
}
