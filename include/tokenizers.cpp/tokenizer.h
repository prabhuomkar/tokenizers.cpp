// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>
#include <vector>

#include "tokenizers.cpp/decoder.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"
#include "tokenizers.cpp/post_processor.h"
#include "tokenizers.cpp/pre_tokenizer.h"

class Encoding {
 public:
  std::vector<int> ids;
  std::vector<int> type_ids;
  std::vector<std::string> tokens;
  std::vector<int> words;
  std::vector<int> special_tokens_mask;
  std::vector<int> attention_mask;
};

class Tokenizer {
 public:
  explicit Tokenizer(std::string path);

  Encoding encode(std::string sequence, bool is_pretokenized = false,
                  bool add_special_tokens = true);
  std::string decode(std::vector<int> ids, bool skip_special_tokens = true);

 private:
  std::string version;
  Normalizer normalizer;
  PreTokenizer pre_tokenizer;
  Model model;
  PostProcessor post_processor;
  Decoder decoder;

  Normalizer with_normalizer(std::string_view type, bool clean_text,
                             bool handle_chinese_chars, bool strip_accents,
                             bool lowercase);
  PreTokenizer with_pre_tokenizer(std::string_view type);
  Model with_model(std::string_view type);
  Decoder with_decoder(std::string_view type);
  PostProcessor with_post_processor(std::string_view type);
};
