// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

enum TRUNCATION_DIRECTION {
  RIGHT_TRUNCATION_DIRECTION,
  LEFT_TRUNCATION_DIRECTION,
  UNKNOWN_TRUNCATION_DIRECTION
};

TRUNCATION_DIRECTION get_truncation_direction(std::string direction);

enum TRUNCATION_STRATEGY {
  LONGEST_FIRST_TRUNCATION_STRATEGY,
  ONLY_FIRST_TRUNCATION_STRATEGY,
  ONLY_SECOND_TRUNCATION_STRATEGY,
  UNKNOWN_TRUNCATION_STRATEGY
};

TRUNCATION_STRATEGY get_truncation_strategy(std::string strategy);

class Truncation {
 public:
  Truncation(const std::string& direction, const std::string& strategy,
             int max_length, int stride);
  Encoding truncate_encoding(Encoding encoding) const;

 private:
  TRUNCATION_DIRECTION direction;
  TRUNCATION_STRATEGY strategy;
  int max_length;
  int stride;
};

enum PADDING_DIRECTION {
  RIGHT_PADDING_DIRECTION,
  LEFT_PADDING_DIRECTION,
  UNKNOWN_PADDING_DIRECTION
};

PADDING_DIRECTION get_padding_direction(std::string direction);

enum PADDING_STRATEGY {
  BATCH_LONGEST_PADDING_STRATEGY,
  FIXED_PADDING_STRATEGY,
  UNKNOWN_PADDING_STRATEGY
};

PADDING_STRATEGY get_padding_strategy(std::string strategy);

class Padding {
 public:
  Padding(const std::string& direction, const std::string& strategy,
          int fixed_size, int pad_id, int pad_type_id,
          const std::string& pad_token, int pad_to_multiple_of);
  Encoding pad_encoding(const Encoding& encoding) const;

 private:
  PADDING_DIRECTION direction;
  PADDING_STRATEGY strategy;
  int fixed_size;
  int pad_id;
  int pad_type_id;
  std::string pad_token;
  int pad_to_multiple_of;
};

std::unique_ptr<Truncation> with_truncation(
    simdjson::ondemand::object truncation_params);

std::unique_ptr<Padding> with_padding(
    simdjson::ondemand::object padding_params);

std::unordered_map<std::string, int> get_map_ints_from_json(
    simdjson::ondemand::object json_object);
