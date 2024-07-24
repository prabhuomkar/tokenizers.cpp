// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tokenizers/added_vocabulary.h"
#include "tokenizers/common.h"
#include "tokenizers/decoder.h"
#include "tokenizers/model.h"
#include "tokenizers/normalizer.h"
#include "tokenizers/post_processor.h"
#include "tokenizers/pre_tokenizer.h"
#include "tokenizers/utils.h"

class Tokenizer {
 public:
  explicit Tokenizer(std::string path = "", std::string config = "");

  Encoding encode(std::wstring sequence, bool add_special_tokens = true);
  std::string decode(std::vector<int> ids, bool skip_special_tokens = true);
  int add_tokens(std::vector<AddedToken> tokens);
  int add_special_tokens(std::vector<AddedToken> tokens);

 private:
  std::string version;
  std::unique_ptr<Truncation> truncation;
  std::unique_ptr<Padding> padding;
  std::unique_ptr<AddedVocabulary> added_vocabulary;
  std::unique_ptr<Normalizer> normalizer;
  std::unique_ptr<PreTokenizer> pre_tokenizer;
  std::unique_ptr<Model> model;
  std::unique_ptr<PostProcessor> post_processor;
  std::unique_ptr<Decoder> decoder;

  Encoding do_tokenize(std::vector<Split> splits, std::optional<int> word_idx,
                       int type_id);
  Encoding do_post_process(Encoding encoding, bool add_special_tokens);
};