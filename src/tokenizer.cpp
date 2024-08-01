// Copyright 2024 Omkar Prabhu
#include "tokenizers/tokenizer.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "simdjson.h"
#include "tokenizers/added_vocabulary.h"
#include "tokenizers/common.h"
#include "tokenizers/decoder.h"
#include "tokenizers/model.h"
#include "tokenizers/normalizer.h"
#include "tokenizers/post_processor.h"
#include "tokenizers/pre_tokenizer.h"
#include "tokenizers/utils.h"

Tokenizer::Tokenizer(const std::string& path, const std::string& config) {
  if (path.length() == 0 && config.length() == 0) {
    throw std::invalid_argument(
        "Requires path or config for initializing a tokenizer!");
  }

  simdjson::ondemand::parser parser;
  simdjson::padded_string tokenizer_json_str;

  try {
    if (path.length() != 0) {
      tokenizer_json_str =
          simdjson::padded_string::load(path + "/tokenizer.json");
    } else {
      tokenizer_json_str = simdjson::padded_string(config);
    }
    simdjson::ondemand::document tokenizer_json =
        parser.iterate(tokenizer_json_str);

    if (!tokenizer_json["truncation"].is_null()) {
      simdjson::ondemand::object truncation_params =
          tokenizer_json["truncation"].get_object();
      truncation = with_truncation(truncation_params);
    }
    if (!tokenizer_json["padding"].is_null()) {
      simdjson::ondemand::object padding_params =
          tokenizer_json["padding"].get_object();
      padding = with_padding(padding_params);
    }
    if (!tokenizer_json["added_tokens"].is_null()) {
      simdjson::ondemand::array added_tokens_params =
          tokenizer_json["added_tokens"].get_array();
      added_vocabulary = with_added_vocabulary(added_tokens_params);
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
  } catch (const std::exception& e) {
    throw std::runtime_error("Error parsing tokenizer config");
  }

  if (added_vocabulary != nullptr) {
    add_tokens(added_vocabulary->added_tokens);
  }
}

Encoding Tokenizer::encode(const std::wstring& sequence,
                           bool add_special_tokens) {
  PreTokenizedString pre_tokenized =
      PreTokenizedString(NormalizedString(sequence));
  if (added_vocabulary != nullptr) {
    pre_tokenized =
        added_vocabulary->extract_and_normalize(normalizer.get(), sequence);
  }
  if (pre_tokenizer != nullptr) {
    pre_tokenized = pre_tokenizer->pre_tokenize(pre_tokenized);
  }
  Encoding encoding = do_tokenize(pre_tokenized, std::nullopt, 0);
  return do_post_process(encoding, add_special_tokens);
}

std::string Tokenizer::decode(const std::vector<int>& ids,
                              bool skip_special_tokens) {
  std::vector<std::string> tokens;
  for (int id : ids) {
    std::string token = "";
    std::optional<std::string> av_token = added_vocabulary->id_to_token(id);
    if (!av_token.has_value()) {
      token = model->id_to_token(id).value();
    } else {
      token = av_token.value();
    }
    if (!skip_special_tokens || !added_vocabulary->is_special_token(token)) {
      tokens.push_back(token);
    }
  }
  tokens = decoder->decode_chain(tokens);
  return std::accumulate(tokens.begin(), tokens.end(), std::string{});
}

int Tokenizer::add_tokens(const std::vector<AddedToken>& tokens) {
  return added_vocabulary->add_tokens(tokens, model.get(), normalizer.get());
}

int Tokenizer::add_special_tokens(const std::vector<AddedToken>& tokens) {
  return added_vocabulary->add_special_tokens(tokens, model.get(),
                                              normalizer.get());
}

Encoding into_encoding(PreTokenizedString pre_tokenized,
                       std::optional<int> word_idx, int type_id) {
  Encoding encoding;
  for (int idx = 0; idx < pre_tokenized.splits.size(); idx++) {
    Split split = pre_tokenized.splits[idx];
    std::pair<int, int> transformed_split_offset = {
        pre_tokenized.normalized
            .offsets[pre_tokenized.normalized.offset_ranges[split.offsets.first]
                         .first]
            .first,
        pre_tokenized.normalized
            .offsets[pre_tokenized.normalized.offset_ranges[split.offsets.first]
                         .first]
            .second};
    for (Token token : split.tokens) {
      encoding.ids.push_back(token.id);
      encoding.tokens.push_back(token.value);
      encoding.offsets.push_back(
          {transformed_split_offset.first + token.offsets.first,
           transformed_split_offset.first + token.offsets.second});
      encoding.words.push_back(word_idx.has_value() ? word_idx.value() : idx);
      encoding.type_ids.push_back(type_id);
      encoding.special_tokens_mask.push_back(0);
      encoding.attention_mask.push_back(1);
    }
  }
  return encoding;
}

Encoding Tokenizer::do_tokenize(PreTokenizedString pre_tokenized,
                                std::optional<int> word_idx,
                                int type_id) const {
  if (model != nullptr) {
    pre_tokenized = model->tokenize(pre_tokenized);
  }
  return into_encoding(pre_tokenized, word_idx, type_id);
}

Encoding Tokenizer::do_post_process(Encoding encoding,
                                    bool add_special_tokens) const {
  if (truncation != nullptr) {
    encoding = truncation->truncate_encoding(encoding);
  }
  if (post_processor != nullptr) {
    encoding = post_processor->process(encoding, add_special_tokens);
  }
  if (padding != nullptr) {
    encoding = padding->pad_encoding(encoding);
  }
  return encoding;
}
