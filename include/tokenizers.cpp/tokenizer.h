// Copyright 2024 Omkar Prabhu
#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tokenizers.cpp/added_vocabulary.h"
#include "tokenizers.cpp/decoder.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"
#include "tokenizers.cpp/post_processor.h"
#include "tokenizers.cpp/pre_tokenizer.h"
#include "tokenizers.cpp/utils.h"

class Encoding {
 public:
  std::vector<int> ids;
  std::vector<int> type_ids;
  std::vector<std::string> tokens;
  std::vector<int> words;
  std::vector<int> special_tokens_mask;
  std::vector<int> attention_mask;
};

class Token {
 public:
  int id;
  std::string value;
  std::pair<int, int> offsets;
  Token(int id, std::string value, std::pair<int, int> offsets);
};

class Tokenizer {
 public:
  explicit Tokenizer(std::string path);

  Encoding encode(std::string sequence, bool is_pretokenized = false,
                  bool add_special_tokens = true);
  std::string decode(std::vector<int> ids, bool skip_special_tokens = true);
  int add_tokens(std::vector<AddedToken> tokens);
  int add_special_tokens(std::vector<AddedToken> tokens);

 private:
  std::string version;
  AddedVocabulary added_vocabulary;
  std::optional<Normalizer> normalizer;
  PreTokenizer pre_tokenizer;
  Model model;
  PostProcessor post_processor;
  Decoder decoder;

  AddedVocabulary with_added_vocabulary(
      AddedVocabularyConfig added_vocabulary_config);
  std::optional<Normalizer> with_normalizer(NormalizerConfig normalizer_config);
  PreTokenizer with_pre_tokenizer(std::string type);
  Model with_model(ModelConfig model_config);
  Decoder with_decoder(DecoderConfig decoder_config);
  PostProcessor with_post_processor(std::string type);
};
