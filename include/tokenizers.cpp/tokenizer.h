// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

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

class Tokenizer {
 public:
  explicit Tokenizer(std::string path);

  Encoding encode(std::string sequence, bool is_pretokenized = false,
                  bool add_special_tokens = true);
  std::string decode(std::vector<int> ids, bool skip_special_tokens = true);

 private:
  std::string version;
  AddedVocabulary added_vocabulary;
  Normalizer normalizer;
  PreTokenizer pre_tokenizer;
  Model model;
  PostProcessor post_processor;
  Decoder decoder;

  AddedVocabulary with_added_vocabulary(
      AddedVocabularyConfig added_vocabulary_config);
  Normalizer with_normalizer(NormalizerConfig normalizer_config);
  PreTokenizer with_pre_tokenizer(std::string type);
  Model with_model(ModelConfig model_config);
  Decoder with_decoder(DecoderConfig decoder_config);
  PostProcessor with_post_processor(std::string type);
};
