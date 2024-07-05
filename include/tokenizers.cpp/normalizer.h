// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

#include "simdjson.h"

enum NORMALIZER {
  BERT_NORMALIZER,
  LOWERCASE_NORMALIZER,
  NFC_NORMALIZER,
  NFD_NORMALIZER,
  NFKC_NORMALIZER,
  NFKD_NORMALIZER,
  NMT_NORMALIZER,
  PRECOMPILED_NORMALIZER,
  REPLACE_NORMALIZER,
  STRIP_NORMALIZER,
  STRIP_ACCENTS_NORMALIZER,
  UNKNOWN_NORMALIZER
};

NORMALIZER get_normalizer(std::string type);

class NormalizerConfig {
 public:
  std::string type;
  bool clean_text;
  bool handle_chinese_chars;
  bool strip_accents;
  bool lowercase;
  explicit NormalizerConfig(simdjson::ondemand::object normalizer_params);
};

class Normalizer {
 public:
  Normalizer();
};

class BertNormalizer : public Normalizer {
 public:
  explicit BertNormalizer(bool clean_text = true,
                          bool handle_chinese_chars = true,
                          bool strip_accents = false, bool lowercase = true);

 private:
  bool clean_text;
  bool handle_chinese_chars;
  bool strip_accents;
  bool lowercase;
};
