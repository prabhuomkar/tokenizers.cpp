// Copyright 2024 Omkar Prabhu
#pragma once

#include <string>

enum POST_PROCESSOR {
  BERT_PROCESSING,
  BYTE_LEVEL_PROCESSING,
  ROBERTA_PROCESSING,
  TEMPLATE_PROCESSING,
  UNKNOWN_POST_PROCESSOR
};

POST_PROCESSOR get_post_processor(std::string_view type);

class PostProcessor {
 public:
  PostProcessor();
};

class TemplateProcessing : public PostProcessor {
 public:
  TemplateProcessing();
};
