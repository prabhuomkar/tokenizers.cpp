// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/normalizer.h"

#include <iostream>
#include <string>
#include <unordered_map>

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

Normalizer::Normalizer() {}

BertNormalizer::BertNormalizer(bool clean_text, bool handle_chinese_chars,
                               bool strip_accents, bool lowercase) {
  std::cout << "Initialized Normalizer: BertNormalizer" << std::endl;
}
