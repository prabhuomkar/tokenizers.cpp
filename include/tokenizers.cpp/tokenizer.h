// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tokenizers.cpp/added_vocabulary.h"
#include "tokenizers.cpp/common.h"
#include "tokenizers.cpp/decoder.h"
#include "tokenizers.cpp/model.h"
#include "tokenizers.cpp/normalizer.h"
#include "tokenizers.cpp/post_processor.h"
#include "tokenizers.cpp/pre_tokenizer.h"
#include "tokenizers.cpp/utils.h"

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
  std::optional<std::unique_ptr<Normalizer>> normalizer;
  PreTokenizer pre_tokenizer;
  Model model;
  PostProcessor post_processor;
  Decoder decoder;

  AddedVocabulary with_added_vocabulary(
      AddedVocabularyConfig added_vocabulary_config);
};
