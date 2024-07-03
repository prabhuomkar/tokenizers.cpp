// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/utils.h"

#include <string>
#include <vector>

Truncation::Truncation() {}

Padding::Padding() {}

AddedToken::AddedToken() {}

std::unordered_map<std::string_view, int> get_map_from_json(
    simdjson::ondemand::object json_object) {
  std::unordered_map<std::string_view, int> result;
  for (auto element : json_object) {
    result.insert(
        {element.escaped_key(), static_cast<int>(element.value().get_int64())});
  }
  return result;
}
