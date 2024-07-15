// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

#include "simdjson.h"

enum POST_PROCESSOR {
  BERT_PROCESSING,
  BYTE_LEVEL_PROCESSING,
  ROBERTA_PROCESSING,
  TEMPLATE_PROCESSING,
  UNKNOWN_POST_PROCESSOR
};

POST_PROCESSOR get_post_processor(std::string type);

class PostProcessor {
 public:
  PostProcessor();
};

PostProcessor with_post_processor(
    simdjson::ondemand::object post_processor_params);

class TemplateProcessing : public PostProcessor {
 public:
  TemplateProcessing();
};
