// Copyright 2024 Omkar Prabhu
#include "tokenizers/post_processor.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "simdjson.h"

POST_PROCESSOR get_post_processor(std::string type) {
  static const std::unordered_map<std::string, POST_PROCESSOR> types = {
      {"BertProcessing", BERT_PROCESSING},
      {"ByteLevel", BYTE_LEVEL_PROCESSING},
      {"RobertaProcessing", ROBERTA_PROCESSING},
      {"TemplateProcessing", TEMPLATE_PROCESSING}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_POST_PROCESSOR;
}

SpecialToken::SpecialToken() {}

SpecialToken::SpecialToken(const std::string& id, const std::vector<int>& ids,
                           const std::vector<std::string>& tokens)
    : id(id), ids(ids), tokens(tokens) {}

Piece::Piece(std::string id, int type_id) : id(id), type_id(type_id) {}

std::unique_ptr<PostProcessor> with_post_processor(
    simdjson::ondemand::object post_processor_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(static_cast<std::string_view>(
      post_processor_params["type"].get_string()));
  if (get_post_processor(type) == TEMPLATE_PROCESSING) {
    val = post_processor_params["single"].value();
    std::vector<std::pair<std::string, Piece>> single;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::array single_array = val.get_array();
      for (auto element : single_array) {
        simdjson::ondemand::object map_piece = element.get_object().value();
        for (auto field : map_piece) {
          type =
              std::string(static_cast<std::string_view>(field.escaped_key()));
          val = field.value();
          std::string id = std::string(
              static_cast<std::string_view>(val["id"].value().get_string()));
          int type_id = static_cast<int>(val["type_id"].value().get_int64());
          single.push_back({type, Piece(id, type_id)});
        }
      }
    }
    val = post_processor_params["pair"].value();
    std::vector<std::pair<std::string, Piece>> pair;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::array pair_array = val.get_array();
      for (auto element : pair_array) {
        simdjson::ondemand::object map_piece = element.get_object().value();
        for (auto field : map_piece) {
          type =
              std::string(static_cast<std::string_view>(field.escaped_key()));
          val = field.value();
          std::string id = std::string(
              static_cast<std::string_view>(val["id"].value().get_string()));
          int type_id = static_cast<int>(val["type_id"].value().get_int64());
          pair.push_back({type, Piece(id, type_id)});
        }
      }
    }
    val = post_processor_params["special_tokens"].value();
    std::vector<SpecialToken> special_tokens;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::object special_tokens_obj = val.get_object();
      for (auto element : special_tokens_obj) {
        val = element.value();
        std::string id = std::string(
            static_cast<std::string_view>(val["id"].value().get_string()));
        std::vector<int> ids;
        auto ids_val = val["ids"].value();
        simdjson::ondemand::array ids_array = ids_val.get_array();
        for (auto id_elem : ids_array) {
          ids.push_back(static_cast<int>(id_elem.value().get_int64()));
        }
        std::vector<std::string> tokens;
        auto tokens_val = val["tokens"].value();
        simdjson::ondemand::array tokens_array = tokens_val.get_array();
        for (auto token_elem : tokens_array) {
          tokens.push_back(std::string(
              static_cast<std::string_view>(token_elem.value().get_string())));
        }
        special_tokens.push_back(SpecialToken(id, ids, tokens));
      }
    }
    return std::make_unique<TemplateProcessing>(
        TemplateProcessing(single, pair, special_tokens));
  }
  return nullptr;
}

TemplateProcessing::TemplateProcessing(
    const std::vector<std::pair<std::string, Piece>>& single,
    const std::vector<std::pair<std::string, Piece>>& pair,
    const std::vector<SpecialToken>& special_tokens)
    : single(single), pair(pair), special_tokens(special_tokens) {}

Encoding TemplateProcessing::process(Encoding encoding,
                                     bool add_special_tokens) const {
  std::vector<Encoding> final_encodings;
  for (auto piece : single) {
    if (piece.first == "Sequence") {
      if (piece.second.id == "A") {
        std::vector<int> type_ids(encoding.type_ids.size(),
                                  piece.second.type_id);
        encoding.type_ids = type_ids;
      }
      final_encodings.push_back(encoding);
    }
    if (piece.first == "SpecialToken") {
      if (add_special_tokens) {
        Encoding new_encoding;
        SpecialToken special_token;
        auto it = std::find_if(special_tokens.begin(), special_tokens.end(),
                               [&piece](const SpecialToken& special_token_it) {
                                 return special_token_it.id == piece.second.id;
                               });

        if (it != special_tokens.end()) {
          special_token = *it;
        }
        new_encoding.ids = special_token.ids;
        new_encoding.tokens = special_token.tokens;
        new_encoding.type_ids =
            std::vector<int>(special_token.ids.size(), piece.second.type_id);
        new_encoding.words = std::vector<std::optional<int>>(
            special_token.ids.size(), std::nullopt);
        new_encoding.offsets =
            std::vector<std::pair<int, int>>(special_token.ids.size(), {0, 0});
        new_encoding.special_tokens_mask =
            std::vector<int>(special_token.ids.size(), 1);
        new_encoding.attention_mask =
            std::vector<int>(special_token.ids.size(), 1);
        final_encodings.push_back(new_encoding);
      }
    }
  }
  Encoding result;
  for (Encoding final_encoding : final_encodings) {
    result.ids.insert(result.ids.end(), final_encoding.ids.begin(),
                      final_encoding.ids.end());
    result.type_ids.insert(result.type_ids.end(),
                           final_encoding.type_ids.begin(),
                           final_encoding.type_ids.end());
    result.tokens.insert(result.tokens.end(), final_encoding.tokens.begin(),
                         final_encoding.tokens.end());
    result.words.insert(result.words.end(), final_encoding.words.begin(),
                        final_encoding.words.end());
    result.offsets.insert(result.offsets.end(), final_encoding.offsets.begin(),
                          final_encoding.offsets.end());
    result.special_tokens_mask.insert(
        result.special_tokens_mask.end(),
        final_encoding.special_tokens_mask.begin(),
        final_encoding.special_tokens_mask.end());
    result.attention_mask.insert(result.attention_mask.end(),
                                 final_encoding.attention_mask.begin(),
                                 final_encoding.attention_mask.end());
  }
  return result;
}
