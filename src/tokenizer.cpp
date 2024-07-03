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
#include "tokenizers.cpp/utils.h"

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
    normalizer = with_normalizer(NormalizerConfig(normalizer_params));
  }
  if (!tokenizer_json["pre_tokenizer"].is_null()) {
    simdjson::ondemand::object pre_tokenizer_params =
        tokenizer_json["pre_tokenizer"].value();
    pre_tokenizer =
        with_pre_tokenizer(pre_tokenizer_params["type"].get_string().value());
  }
  if (!tokenizer_json["model"].is_null()) {
    simdjson::ondemand::object model_params = tokenizer_json["model"].value();
    model = with_model(ModelConfig(model_params));
  }
  if (!tokenizer_json["decoder"].is_null()) {
    simdjson::ondemand::object decoder_params =
        tokenizer_json["decoder"].value();
    decoder = with_decoder(DecoderConfig(decoder_params));
  }
  if (!tokenizer_json["post_processor"].is_null()) {
    simdjson::ondemand::object post_processor_params =
        tokenizer_json["post_processor"].value();
    post_processor =
        with_post_processor(post_processor_params["type"].get_string().value());
  }
}

Encoding Tokenizer::encode(std::string sequence, bool is_pretokenized,
                           bool add_special_tokens) {
  return Encoding();
}

std::string Tokenizer::decode(std::vector<int> ids, bool skip_special_tokens) {
  return "";
}

Normalizer Tokenizer::with_normalizer(NormalizerConfig normalizer_config) {
  switch (get_normalizer(normalizer_config.type)) {
    case BERT_NORMALIZER:
      return BertNormalizer(
          normalizer_config.clean_text, normalizer_config.handle_chinese_chars,
          normalizer_config.strip_accents, normalizer_config.lowercase);
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

Model Tokenizer::with_model(ModelConfig model_config) {
  switch (get_model(model_config.type)) {
    case WORD_PIECE_MODEL:
      return WordPiece(model_config.vocab, model_config.unk_token,
                       model_config.max_input_chars_per_word);
    default:
      break;
  }

  return Model();
}

Decoder Tokenizer::with_decoder(DecoderConfig decoder_config) {
  switch (get_decoder(decoder_config.type)) {
    case WORD_PIECE_DECODER:
      return WordPieceDecoder(decoder_config.prefix, decoder_config.cleanup);
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
