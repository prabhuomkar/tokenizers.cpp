// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/post_processor.h"

#include <iostream>
#include <string>
#include <unordered_map>

POST_PROCESSOR get_post_processor(std::string type) {
  static const std::unordered_map<std::string, POST_PROCESSOR> types = {
      {"BertProcessing", BERT_PROCESSING},
      {"ByteLevel", BYTE_LEVEL_PROCESSING},
      {"RobertaProcessing", ROBERTA_PROCESSING},
      {"TemplateProcessing", TEMPLATE_PROCESSING}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_POST_PROCESSOR;
}

PostProcessor::PostProcessor() {}

TemplateProcessing::TemplateProcessing() {
  std::cout << "Initialized PostProcessor: TemplateProcessing" << std::endl;
}
