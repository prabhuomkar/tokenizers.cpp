// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/normalizer.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include "simdjson.h"

NORMALIZER get_normalizer(std::string_view type) {
  static const std::unordered_map<std::string_view, NORMALIZER> types = {
      {"BertNormalizer", BERT_NORMALIZER},
      {"Lowercase", LOWERCASE_NORMALIZER},
      {"NFC", NFC_NORMALIZER},
      {"NFD", NFD_NORMALIZER},
      {"NFKC", NFKC_NORMALIZER},
      {"NFKD", NFKD_NORMALIZER},
      {"Precompiled", PRECOMPILED_NORMALIZER},
      {"Replace", REPLACE_NORMALIZER},
      {"Strip", STRIP_NORMALIZER},
      {"StripAccents", STRIP_ACCENTS_NORMALIZER}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_NORMALIZER;
}

NormalizerConfig::NormalizerConfig(
    simdjson::ondemand::object normalizer_params) {
  simdjson::ondemand::value val;
  type = normalizer_params["type"].get_string();

  val = normalizer_params["clean_text"].value();
  clean_text = val.type() == simdjson::ondemand::json_type::null
                   ? false
                   : static_cast<bool>(val.get_bool());

  val = normalizer_params["handle_chinese_chars"].value();
  handle_chinese_chars = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());
  val = normalizer_params["strip_accents"].value();
  strip_accents = val.type() == simdjson::ondemand::json_type::null
                      ? false
                      : static_cast<bool>(val.get_bool());

  val = normalizer_params["lowercase"].value();
  lowercase = val.type() == simdjson::ondemand::json_type::null
                  ? false
                  : static_cast<bool>(val.get_bool());
}

Normalizer::Normalizer() {}

BertNormalizer::BertNormalizer(bool clean_text, bool handle_chinese_chars,
                               bool strip_accents, bool lowercase)
    : clean_text(clean_text),
      handle_chinese_chars(handle_chinese_chars),
      strip_accents(strip_accents),
      lowercase(lowercase) {
  std::cout << "Initialized Normalizer: BertNormalizer" << std::endl;
  std::cout << "params:" << clean_text << " " << handle_chinese_chars << " "
            << strip_accents << " " << lowercase << std::endl;
}
