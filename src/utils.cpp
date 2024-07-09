// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/utils.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

Truncation::Truncation() {}

Padding::Padding() {}

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
