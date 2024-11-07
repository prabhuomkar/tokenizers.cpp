// Copyright 2024 Omkar Prabhu
#include "tokenizers/post_processor.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simdjson.h"

POST_PROCESSOR get_post_processor(std::string type) {
  static const std::unordered_map<std::string, POST_PROCESSOR> types = {
      {"BertProcessing", BERT_PROCESSING},
      {"ByteLevel", BYTE_LEVEL_PROCESSING},
      {"RobertaProcessing", ROBERTA_PROCESSING},
      {"TemplateProcessing", TEMPLATE_PROCESSING},
      {"Sequence", SEQUENCE_PROCESSING}};

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
  } else if (get_post_processor(type) == SEQUENCE_PROCESSING) {
    simdjson::ondemand::array seq_processors_list =
        post_processor_params["processors"].get_array();
    std::vector<std::unique_ptr<PostProcessor>> seq_processors;
    for (simdjson::ondemand::value processor_val : seq_processors_list) {
      simdjson::ondemand::object seq_processor_params =
          processor_val.get_object();
      std::unique_ptr<PostProcessor> seq_processor =
          with_post_processor(seq_processor_params);
      if (seq_processor != nullptr) {
        seq_processors.push_back(std::move(seq_processor));
      }
    }
    return std::make_unique<SequenceProcessing>(
        SequenceProcessing(std::move(seq_processors)));
  } else if (get_post_processor(type) == BYTE_LEVEL_PROCESSING) {
    val = post_processor_params["add_prefix_space"].value();
    bool add_prefix_space = val.type() == simdjson::ondemand::json_type::null
                                ? false
                                : static_cast<bool>(val.get_bool());
    val = post_processor_params["trim_offsets"].value();
    bool trim_offsets = val.type() == simdjson::ondemand::json_type::null
                            ? false
                            : static_cast<bool>(val.get_bool());
    return std::make_unique<ByteLevelProcessing>(
        ByteLevelProcessing(add_prefix_space, trim_offsets));
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

Encoding ByteLevelProcessing::process_offsets(const Encoding& encoding,
                                              bool add_prefix_space) const {
  Encoding new_encoding = encoding;
  for (int i = 0; i < encoding.ids.size(); i++) {
    std::string token = encoding.tokens[i];
    std::pair<int, int> offsets = encoding.offsets[i];
    int leading_spaces = 0, trailing_spaces = 0;
    icu::UnicodeString unicode_token = icu::UnicodeString::fromUTF8(token);
    for (int j = 0; j < unicode_token.length(); j++) {
      std::string token_char;
      unicode_token.tempSubString(j, 1).toUTF8String(token_char);
      if (BYTES_CHAR.at(static_cast<uint16_t>(' ')) == token_char) {
        leading_spaces++;
      } else {
        break;
      }
    }
    for (int j = unicode_token.length() - 1; j >= 0; j--) {
      std::string token_char;
      unicode_token.tempSubString(j, 1).toUTF8String(token_char);
      if (BYTES_CHAR.at(static_cast<uint16_t>(' ')) == token_char) {
        trailing_spaces++;
      } else {
        break;
      }
    }
    if (leading_spaces > 0 || trailing_spaces > 0) {
      if (leading_spaces > 0) {
        bool is_first = (i == 0 || offsets.first == 0);
        if (is_first && add_prefix_space && leading_spaces == 1) {
          leading_spaces = 0;
        }
        offsets.first =
            std::min(offsets.first + leading_spaces, offsets.second);
      }
      if (trailing_spaces > 0 && offsets.second >= trailing_spaces) {
        offsets.second =
            std::max(offsets.second - trailing_spaces, offsets.first);
      }
    }
    new_encoding.tokens[i] = token;
    new_encoding.offsets[i] = offsets;
  }
  return new_encoding;
}

ByteLevelProcessing::ByteLevelProcessing(bool add_prefix_space,
                                         bool trim_offsets)
    : add_prefix_space(add_prefix_space),
      trim_offsets(trim_offsets),
      BYTES_CHAR(bytes_char()) {}

Encoding ByteLevelProcessing::process(Encoding encoding,
                                      bool add_special_tokens) const {
  if (trim_offsets) {
    encoding = process_offsets(encoding, add_prefix_space);
    for (Encoding& overflowing_encoding : encoding.overflowing) {
      overflowing_encoding =
          process_offsets(overflowing_encoding, add_prefix_space);
    }
  }
  return encoding;
}

SequenceProcessing::SequenceProcessing(
    std::vector<std::unique_ptr<PostProcessor>> processors)
    : processors(std::move(processors)) {}

Encoding SequenceProcessing::process(Encoding encoding,
                                     bool add_special_tokens) const {
  encoding = std::accumulate(
      processors.begin(), processors.end(), encoding,
      [add_special_tokens](const Encoding& acc,
                           const std::unique_ptr<PostProcessor>& processor) {
        return processor->process(acc, add_special_tokens);
      });
  return encoding;
}
