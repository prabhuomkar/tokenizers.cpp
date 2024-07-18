// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/utils.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

Truncation::Truncation(std::string direction, std::string strategy,
                       int max_length, int stride)
    : direction(direction),
      strategy(strategy),
      max_length(max_length),
      stride(stride) {}

Padding::Padding(std::string direction, std::string strategy, int pad_id,
                 int pad_type_id, std::string pad_token, int pad_to_multiple_of)
    : direction(direction),
      strategy(strategy),
      pad_id(pad_id),
      pad_type_id(pad_type_id),
      pad_token(pad_token),
      pad_to_multiple_of(pad_to_multiple_of) {}

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

std::unique_ptr<Padding> with_padding(
    simdjson::ondemand::object padding_params) {
  simdjson::ondemand::value val;
  val = padding_params["direction"].value();
  std::string direction =
      std::string(val.type() == simdjson::ondemand::json_type::null
                      ? ""
                      : static_cast<std::string_view>(val.get_string()));
  val = padding_params["strategy"].value();
  std::string strategy =
      std::string(val.type() == simdjson::ondemand::json_type::null
                      ? ""
                      : static_cast<std::string_view>(val.get_string()));
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
  return std::make_unique<Padding>(Padding(
      direction, strategy, pad_id, pad_type_id, pad_token, pad_to_multiple_of));
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
