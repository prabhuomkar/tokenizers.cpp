// Copyright 2024 Omkar Prabhu
#pragma once

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

enum NORMALIZER {
  SEQUENCE_NORMALIZER,
  BERT_NORMALIZER,
  LOWERCASE_NORMALIZER,
  PREPEND_NORMALIZER,
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

class NormalizedString {
 public:
  std::wstring normalized;
  std::vector<std::pair<int, int>> offsets;
  std::vector<std::pair<int, int>> offset_ranges;
  explicit NormalizedString(std::wstring normalized);
  NormalizedString(std::wstring normalized,
                   std::vector<std::pair<int, int>> offsets);
  void transform(int i, std::string op, int n);
};

class Normalizer {
 public:
  virtual ~Normalizer() = default;
  virtual NormalizedString normalize(NormalizedString normalized) const = 0;
};

std::unique_ptr<Normalizer> with_normalizer(
    simdjson::ondemand::object normalizer_params);

class BertNormalizer : public Normalizer {
 public:
  explicit BertNormalizer(bool clean_text = true,
                          bool handle_chinese_chars = true,
                          bool strip_accents = false, bool lowercase = true);
  NormalizedString normalize(NormalizedString normalized) const override;

 private:
  bool clean_text;
  bool handle_chinese_chars;
  bool strip_accents;
  bool lowercase;
  NormalizedString do_clean_text(NormalizedString normalized) const;
  NormalizedString do_handle_chinese_chars(NormalizedString normalized) const;
  NormalizedString do_strip_accents(NormalizedString normalized) const;
  NormalizedString do_lowercase(NormalizedString normalized) const;
};

class Prepend : public Normalizer {
 public:
  explicit Prepend(std::string prepend);
  NormalizedString normalize(NormalizedString normalized) const override;

 private:
  std::string prepend;
};

class Replace : public Normalizer {
 public:
  explicit Replace(std::string pattern, std::string content);
  NormalizedString normalize(NormalizedString normalized) const override;

 private:
  std::string pattern;
  std::string content;
};

class NFD : public Normalizer {
 public:
  NFD();
  NormalizedString normalize(NormalizedString normalized) const override;
};

class SequenceNormalizer : public Normalizer {
 public:
  explicit SequenceNormalizer(
      std::vector<std::unique_ptr<Normalizer>> normalizers);
  NormalizedString normalize(NormalizedString normalized) const override;

 private:
  std::vector<std::unique_ptr<Normalizer>> normalizers;
};
