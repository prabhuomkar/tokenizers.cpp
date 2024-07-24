// Copyright 2024 Omkar Prabhu
#include "tokenizers/utils.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

TRUNCATION_DIRECTION get_truncation_direction(std::string direction) {
  static const std::unordered_map<std::string, TRUNCATION_DIRECTION>
      directions = {{"Right", RIGHT_TRUNCATION_DIRECTION},
                    {"Left", LEFT_TRUNCATION_DIRECTION}};

  auto it = directions.find(direction);
  if (it != directions.end()) {
    return it->second;
  }
  return UNKNOWN_TRUNCATION_DIRECTION;
}

TRUNCATION_STRATEGY get_truncation_strategy(std::string strategy) {
  static const std::unordered_map<std::string, TRUNCATION_STRATEGY> strategies =
      {{"LongestFirst", LONGEST_FIRST_TRUNCATION_STRATEGY},
       {"OnlyFirst", ONLY_FIRST_TRUNCATION_STRATEGY},
       {"OnlySecond", ONLY_SECOND_TRUNCATION_STRATEGY}};

  auto it = strategies.find(strategy);
  if (it != strategies.end()) {
    return it->second;
  }
  return UNKNOWN_TRUNCATION_STRATEGY;
}

Truncation::Truncation(std::string direction, std::string strategy,
                       int max_length, int stride)
    : direction(get_truncation_direction(direction)),
      strategy(get_truncation_strategy(strategy)),
      max_length(max_length),
      stride(stride) {}

Encoding truncate(Encoding encoding, int max_length, int stride,
                  TRUNCATION_DIRECTION direction) {
  int encoding_length = encoding.ids.size();
  if (max_length >= encoding_length) {
    return encoding;
  }
  if (max_length == 0) {
    encoding.overflowing = {};
    return encoding;
  }
  int offset = max_length - stride;
  bool end = false;
  std::vector<std::pair<int, int>> parts_ranges;
  if (direction == RIGHT_TRUNCATION_DIRECTION) {
    for (int start = 0; start < encoding.ids.size(); start += offset) {
      if (!end) {
        int stop = std::min(start + max_length, encoding_length);
        end = stop == encoding_length;
        parts_ranges.push_back({start, stop});
      }
    }
  } else if (direction == LEFT_TRUNCATION_DIRECTION) {
    for (int stop = encoding.ids.size() - 1; stop >= 0; stop -= offset) {
      stop += 1;
      int start = stop < max_length ? 0 : stop - max_length;
      end = start == 0;
      parts_ranges.push_back({start, stop});
    }
  }
  std::pair<int, int> elem = parts_ranges[0];
  Encoding new_encoding(
      std::vector<int>(encoding.ids.begin() + elem.first,
                       encoding.ids.begin() + elem.second),
      std::vector<int>(encoding.type_ids.begin() + elem.first,
                       encoding.type_ids.begin() + elem.second),
      std::vector<std::string>(encoding.tokens.begin() + elem.first,
                               encoding.tokens.begin() + elem.second),
      std::vector<std::optional<int>>(encoding.words.begin() + elem.first,
                                      encoding.words.begin() + elem.second),
      std::vector<std::pair<int, int>>(encoding.offsets.begin() + elem.first,
                                       encoding.offsets.begin() + elem.second),
      std::vector<int>(encoding.special_tokens_mask.begin() + elem.first,
                       encoding.special_tokens_mask.begin() + elem.second),
      std::vector<int>(encoding.attention_mask.begin() + elem.first,
                       encoding.attention_mask.begin() + elem.second));
  new_encoding.overflowing = {};
  for (int i = 1; i < parts_ranges.size(); i++) {
    elem = parts_ranges[i];
    new_encoding.overflowing.push_back(Encoding(
        std::vector<int>(encoding.ids.begin() + elem.first,
                         encoding.ids.begin() + elem.second),
        std::vector<int>(encoding.type_ids.begin() + elem.first,
                         encoding.type_ids.begin() + elem.second),
        std::vector<std::string>(encoding.tokens.begin() + elem.first,
                                 encoding.tokens.begin() + elem.second),
        std::vector<std::optional<int>>(encoding.words.begin() + elem.first,
                                        encoding.words.begin() + elem.second),
        std::vector<std::pair<int, int>>(
            encoding.offsets.begin() + elem.first,
            encoding.offsets.begin() + elem.second),
        std::vector<int>(encoding.special_tokens_mask.begin() + elem.first,
                         encoding.special_tokens_mask.begin() + elem.second),
        std::vector<int>(encoding.attention_mask.begin() + elem.first,
                         encoding.attention_mask.begin() + elem.second)));
  }
  return new_encoding;
}

Encoding Truncation::truncate_encoding(Encoding encoding) {
  if (max_length == 0) {
    return truncate(encoding, 0, stride, direction);
  }
  if (encoding.ids.size() <= max_length) {
    return encoding;
  }
  int to_remove =
      encoding.ids.size() > max_length ? encoding.ids.size() - max_length : 0;
  if (strategy == LONGEST_FIRST_TRUNCATION_STRATEGY) {
    encoding =
        truncate(encoding, encoding.ids.size() - to_remove, stride, direction);
  } else if (strategy == ONLY_FIRST_TRUNCATION_STRATEGY) {
    int target_len = encoding.ids.size();
    if (target_len > to_remove) {
      encoding = truncate(encoding, max_length, stride, direction);
    }
  }
  return encoding;
}

std::unique_ptr<Truncation> with_truncation(
    simdjson::ondemand::object truncation_params) {
  simdjson::ondemand::value val;
  val = truncation_params["direction"].value();
  std::string direction =
      std::string(val.type() == simdjson::ondemand::json_type::null
                      ? ""
                      : static_cast<std::string_view>(val.get_string()));
  val = truncation_params["strategy"].value();
  std::string strategy =
      std::string(val.type() == simdjson::ondemand::json_type::null
                      ? ""
                      : static_cast<std::string_view>(val.get_string()));
  val = truncation_params["max_length"].value();
  int max_length = val.type() == simdjson::ondemand::json_type::null
                       ? 0
                       : static_cast<int>(val.get_int64());
  val = truncation_params["stride"].value();
  int stride = val.type() == simdjson::ondemand::json_type::null
                   ? 0
                   : static_cast<int>(val.get_int64());
  return std::make_unique<Truncation>(
      Truncation(direction, strategy, max_length, stride));
}

PADDING_DIRECTION get_padding_direction(std::string direction) {
  static const std::unordered_map<std::string, PADDING_DIRECTION> directions = {
      {"Right", RIGHT_PADDING_DIRECTION}, {"Left", LEFT_PADDING_DIRECTION}};

  auto it = directions.find(direction);
  if (it != directions.end()) {
    return it->second;
  }
  return UNKNOWN_PADDING_DIRECTION;
}

PADDING_STRATEGY get_padding_strategy(std::string strategy) {
  static const std::unordered_map<std::string, PADDING_STRATEGY> strategies = {
      {"BatchLongest", BATCH_LONGEST_PADDING_STRATEGY},
      {"Fixed", FIXED_PADDING_STRATEGY}};

  auto it = strategies.find(strategy);
  if (it != strategies.end()) {
    return it->second;
  }
  return UNKNOWN_PADDING_STRATEGY;
}

Padding::Padding(std::string direction, std::string strategy, int fixed_size,
                 int pad_id, int pad_type_id, std::string pad_token,
                 int pad_to_multiple_of)
    : direction(get_padding_direction(direction)),
      strategy(get_padding_strategy(strategy)),
      fixed_size(fixed_size),
      pad_id(pad_id),
      pad_type_id(pad_type_id),
      pad_token(pad_token),
      pad_to_multiple_of(pad_to_multiple_of) {}

Encoding pad(Encoding encoding, int target_length, int pad_id, int pad_type_id,
             std::string pad_token, PADDING_DIRECTION direction) {
  for (Encoding& overflow_encoding : encoding.overflowing) {
    overflow_encoding = pad(overflow_encoding, target_length, pad_id,
                            pad_type_id, pad_token, direction);
  }
  if (encoding.ids.size() >= target_length) {
    return encoding;
  }
  int pad_length = target_length - encoding.ids.size();
  if (direction == LEFT_PADDING_DIRECTION) {
    encoding.ids.insert(encoding.ids.begin(), pad_length, pad_id);
    encoding.type_ids.insert(encoding.type_ids.begin(), pad_length,
                             pad_type_id);
    encoding.tokens.insert(encoding.tokens.begin(), pad_length, pad_token);
    encoding.words.insert(encoding.words.begin(), pad_length, 0);
    encoding.attention_mask.insert(encoding.attention_mask.begin(), pad_length,
                                   0);
    encoding.special_tokens_mask.insert(encoding.special_tokens_mask.begin(),
                                        pad_length, 1);
    encoding.offsets.insert(encoding.offsets.begin(), pad_length, {0, 0});
  } else if (direction == RIGHT_PADDING_DIRECTION) {
    encoding.ids.insert(encoding.ids.end(), pad_length, pad_id);
    encoding.type_ids.insert(encoding.type_ids.end(), pad_length, pad_type_id);
    encoding.tokens.insert(encoding.tokens.end(), pad_length, pad_token);
    encoding.words.insert(encoding.words.end(), pad_length, 0);
    encoding.attention_mask.insert(encoding.attention_mask.end(), pad_length,
                                   0);
    encoding.special_tokens_mask.insert(encoding.special_tokens_mask.end(),
                                        pad_length, 1);
    encoding.offsets.insert(encoding.offsets.end(), pad_length, {0, 0});
  }
  return encoding;
}

Encoding Padding::pad_encoding(Encoding encoding) {
  int pad_length =
      strategy == FIXED_PADDING_STRATEGY ? fixed_size : encoding.ids.size();
  if (pad_to_multiple_of > 0 && pad_length % pad_to_multiple_of > 0) {
    pad_length += pad_to_multiple_of - pad_length % pad_to_multiple_of;
  }
  return pad(encoding, pad_length, pad_id, pad_type_id, pad_token, direction);
}

std::unique_ptr<Padding> with_padding(
    simdjson::ondemand::object padding_params) {
  simdjson::ondemand::value val;
  val = padding_params["direction"].value();
  std::string direction =
      std::string(val.type() == simdjson::ondemand::json_type::null
                      ? ""
                      : static_cast<std::string_view>(val.get_string()));
  val = padding_params["strategy"].value();
  int fixed_size = -1;
  std::string strategy;
  if (val.type() != simdjson::ondemand::json_type::null) {
    if (val.type() == simdjson::ondemand::json_type::object) {
      simdjson::ondemand::object strategy_obj = val.get_object().value();
      for (auto field : strategy_obj) {
        strategy =
            std::string(static_cast<std::string_view>(field.escaped_key()));
        fixed_size = static_cast<int>(field.value().get_int64());
      }
    } else {
      strategy = std::string(static_cast<std::string_view>(val.get_string()));
    }
  }
  val = padding_params["pad_id"].value();
  int pad_id = val.type() == simdjson::ondemand::json_type::null
                   ? 0
                   : static_cast<int>(val.get_int64());
  val = padding_params["pad_type_id"].value();
  int pad_type_id = val.type() == simdjson::ondemand::json_type::null
                        ? 0
                        : static_cast<int>(val.get_int64());
  val = padding_params["pad_token"].value();
  std::string pad_token =
      std::string(val.type() == simdjson::ondemand::json_type::null
                      ? ""
                      : static_cast<std::string_view>(val.get_string()));
  val = padding_params["pad_to_multiple_of"].value();
  int pad_to_multiple_of = val.type() == simdjson::ondemand::json_type::null
                               ? 0
                               : static_cast<int>(val.get_int64());
  return std::make_unique<Padding>(Padding(direction, strategy, fixed_size,
                                           pad_id, pad_type_id, pad_token,
                                           pad_to_multiple_of));
}

std::unordered_map<std::string, int> get_map_ints_from_json(
    simdjson::ondemand::object json_object) {
  std::unordered_map<std::string, int> result;
  for (auto element : json_object) {
    result.insert(
        {std::string(static_cast<std::string_view>(element.escaped_key())),
         static_cast<int>(element.value().get_int64())});
  }
  return result;
}
