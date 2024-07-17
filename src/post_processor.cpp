// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/post_processor.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

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

SpecialToken::SpecialToken(std::string id, std::vector<int> ids,
                           std::vector<std::string> tokens)
    : id(id), ids(ids), tokens(tokens) {}

Piece::Piece(std::string id, int type_id) : id(id), type_id(type_id) {}

std::unique_ptr<PostProcessor> with_post_processor(
    simdjson::ondemand::object post_processor_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(static_cast<std::string_view>(
      post_processor_params["type"].get_string()));
  if (get_post_processor(type) == TEMPLATE_PROCESSING) {
    val = post_processor_params["single"].value();
    std::vector<std::unordered_map<std::string, Piece>> single;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::array single_array = val.get_array();
      for (auto element : single_array) {
        simdjson::ondemand::object map_piece = element.get_object().value();
        for (auto field : map_piece) {
          std::string type =
              std::string(static_cast<std::string_view>(field.escaped_key()));
          val = field.value();
          std::string id = std::string(
              static_cast<std::string_view>(val["id"].value().get_string()));
          int type_id = static_cast<int>(val["type_id"].value().get_int64());
          single.push_back({{type, Piece(id, type_id)}});
        }
      }
    }
    val = post_processor_params["pair"].value();
    std::vector<std::unordered_map<std::string, Piece>> pair;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::array pair_array = val.get_array();
      for (auto element : pair_array) {
        simdjson::ondemand::object map_piece = element.get_object().value();
        for (auto field : map_piece) {
          std::string type =
              std::string(static_cast<std::string_view>(field.escaped_key()));
          val = field.value();
          std::string id = std::string(
              static_cast<std::string_view>(val["id"].value().get_string()));
          int type_id = static_cast<int>(val["type_id"].value().get_int64());
          pair.push_back({{type, Piece(id, type_id)}});
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
    std::vector<std::unordered_map<std::string, Piece>> single,
    std::vector<std::unordered_map<std::string, Piece>> pair,
    std::vector<SpecialToken> special_tokens)
    : single(single), pair(pair), special_tokens(special_tokens) {}

Encoding TemplateProcessing::post_process(Encoding encoding,
                                          bool add_special_tokens) const {
  return Encoding();
}
