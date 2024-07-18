// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "simdjson.h"

class Truncation {
 public:
  Truncation(std::string direction, std::string strategy, int max_length,
             int stride);

 private:
  std::string direction;
  std::string strategy;
  int max_length;
  int stride;
};

class Padding {
 public:
  Padding(std::string direction, std::string strategy, int pad_id,
          int pad_type_id, std::string pad_token, int pad_to_multiple_of);

 private:
  std::string direction;
  std::string strategy;
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
