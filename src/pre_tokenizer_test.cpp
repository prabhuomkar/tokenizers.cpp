// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/pre_tokenizer.h"

#include <gtest/gtest.h>

#include <memory>
#include <typeinfo>

#include "simdjson.h"

std::unique_ptr<PreTokenizer> get_pre_tokenizer_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object pre_tokenizer_params = doc.get_object().value();
  return with_pre_tokenizer(pre_tokenizer_params);
}

TEST(BertPreTokenizerTest, CleanText) {
  std::unique_ptr<PreTokenizer> pre_tokenizer = get_pre_tokenizer_from_string(
      "{\"type\":\"BertPreTokenizer\",\"clean_text\":true,\"handle_chinese_"
      "chars\":false,\"strip_accents\":null,\"lowercase\":false}");
  EXPECT_NE(pre_tokenizer, nullptr);
  std::string got =
      pre_tokenizer->pre_tokenize(L"Hello World!\tThis is a test.\n");
  EXPECT_EQ("HelloWorld!Thisisatest.", got);
}
