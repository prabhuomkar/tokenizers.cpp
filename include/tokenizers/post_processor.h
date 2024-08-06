// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

enum POST_PROCESSOR {
  BERT_PROCESSING,
  BYTE_LEVEL_PROCESSING,
  ROBERTA_PROCESSING,
  TEMPLATE_PROCESSING,
  SEQUENCE_PROCESSING,
  UNKNOWN_POST_PROCESSOR
};

POST_PROCESSOR get_post_processor(std::string type);

class SpecialToken {
 public:
  std::string id;
  std::vector<int> ids;
  std::vector<std::string> tokens;
  SpecialToken();
  SpecialToken(const std::string& id, const std::vector<int>& ids,
               const std::vector<std::string>& tokens);
};

class Piece {
 public:
  std::string id;
  int type_id;
  Piece(std::string id, int type_id);
};

class PostProcessor {
 public:
  virtual ~PostProcessor() = default;
  virtual Encoding process(Encoding encoding,
                           bool add_special_tokens) const = 0;
};

std::unique_ptr<PostProcessor> with_post_processor(
    simdjson::ondemand::object post_processor_params);

class TemplateProcessing : public PostProcessor {
 public:
  TemplateProcessing(const std::vector<std::pair<std::string, Piece>>& single,
                     const std::vector<std::pair<std::string, Piece>>& pair,
                     const std::vector<SpecialToken>& special_tokens);
  Encoding process(Encoding encoding, bool add_special_tokens) const override;

 private:
  std::vector<std::pair<std::string, Piece>> single;
  std::vector<std::pair<std::string, Piece>> pair;
  std::vector<SpecialToken> special_tokens;
};

class ByteLevelProcessing : public PostProcessor {
 public:
  explicit ByteLevelProcessing(bool add_prefix_space, bool trim_offsets);
  Encoding process(Encoding encoding, bool add_special_tokens) const override;

 private:
  bool add_prefix_space;
  bool trim_offsets;
  std::unordered_map<uint16_t, std::string> BYTES_CHAR;
  Encoding process_offsets(const Encoding& encoding,
                           bool add_prefix_space) const;
};

class SequenceProcessing : public PostProcessor {
 public:
  explicit SequenceProcessing(
      std::vector<std::unique_ptr<PostProcessor>> processors);
  Encoding process(Encoding encoding, bool add_special_tokens) const override;

 private:
  std::vector<std::unique_ptr<PostProcessor>> processors;
};
