// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/post_processor.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include "simdjson.h"

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

PostProcessor with_post_processor(
    simdjson::ondemand::object post_processor_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(static_cast<std::string_view>(
      post_processor_params["type"].get_string()));
  if (get_post_processor(type) == TEMPLATE_PROCESSING) {
    return TemplateProcessing();
  }
  return PostProcessor();
}

TemplateProcessing::TemplateProcessing() {
  std::cout << "Initialized PostProcessor: TemplateProcessing" << std::endl;
}
