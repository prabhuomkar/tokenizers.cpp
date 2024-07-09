// Copyright 2024 Omkar Prabhu
#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "simdjson.h"

class Truncation {
 public:
  Truncation();

 private:
  std::string direction;
  std::string strategy;
  int max_length;
  int stride;
};

class Padding {
 public:
  Padding();

 private:
  std::string direction;
  std::string strategy;
  int pad_id;
  int pad_type_id;
  std::string pad_token;
  int pad_to_multiple_of;
};

std::unordered_map<std::string, int> get_map_ints_from_json(
    simdjson::ondemand::object json_object);
