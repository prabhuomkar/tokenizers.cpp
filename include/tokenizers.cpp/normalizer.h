// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

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

NORMALIZER get_normalizer(std::string_view type);

class Normalizer {
 public:
  Normalizer();
};

class BertNormalizer : public Normalizer {
 public:
  BertNormalizer(bool clean_text, bool handle_chinese_chars, bool strip_accents,
                 bool lowercase);
};
