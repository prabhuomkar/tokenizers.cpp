// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>
#include <unordered_map>

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

class AddedToken {
 public:
  AddedToken();

 private:
  int id;
  std::string content;
  bool single_word;
  bool lstrip;
  bool rstrip;
  bool normalized;
  bool special;
};

std::unordered_map<std::string_view, int> get_map_from_json(
    simdjson::ondemand::object json_object);
