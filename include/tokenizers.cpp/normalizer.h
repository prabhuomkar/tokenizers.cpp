// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "simdjson.h"

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

class Normalizer {
 public:
  virtual ~Normalizer() = default;
  virtual std::wstring normalize(std::wstring normalized) const = 0;
};

std::optional<std::unique_ptr<Normalizer>> with_normalizer(
    simdjson::ondemand::object normalizer_params);

class BertNormalizer : public Normalizer {
 public:
  explicit BertNormalizer(bool clean_text = true,
                          bool handle_chinese_chars = true,
                          bool strip_accents = false, bool lowercase = true);
  std::wstring normalize(std::wstring normalized) const override;

 private:
  bool clean_text;
  bool handle_chinese_chars;
  bool strip_accents;
  bool lowercase;
  std::wstring do_clean_text(std::wstring normalized) const;
  std::wstring do_handle_chinese_chars(std::wstring normalized) const;
  std::wstring do_strip_accents(std::wstring normalized) const;
  std::wstring do_lowercase(std::wstring normalized) const;
};

class Prepend : public Normalizer {
 public:
  explicit Prepend(std::string prepend);
  std::wstring normalize(std::wstring normalized) const override;

 private:
  std::string prepend;
};

class Replace : public Normalizer {
 public:
  explicit Replace(std::string pattern, std::string content);
  std::wstring normalize(std::wstring normalized) const override;

 private:
  std::string pattern;
  std::string content;
};

class NFD : public Normalizer {
 public:
  NFD();
  std::wstring normalize(std::wstring normalized) const override;
};

class SequenceNormalizer : public Normalizer {
 public:
  explicit SequenceNormalizer(
      std::vector<std::unique_ptr<Normalizer>> normalizers);
  std::wstring normalize(std::wstring normalized) const override;

 private:
  std::vector<std::unique_ptr<Normalizer>> normalizers;
};
