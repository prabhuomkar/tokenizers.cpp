// Copyright 2024 Omkar Prabhu
#include "tokenizers/added_vocabulary.h"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "simdjson.h"
#include "tokenizers/model.h"

std::unique_ptr<AddedVocabulary> get_added_vocabulary_from_string(
    std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::array added_vocabulary_params = doc.get_array().value();
  return with_added_vocabulary(added_vocabulary_params);
}

void validate_added_vocabulary_splits(std::vector<Split> expected,
                                      std::vector<Split> got) {
  EXPECT_EQ(expected.size(), got.size());
  for (int i = 0; i < expected.size(); i++) {
    EXPECT_EQ(expected[i].normalized, got[i].normalized);
    EXPECT_EQ(expected[i].offsets.first, got[i].offsets.first);
    EXPECT_EQ(expected[i].offsets.second, got[i].offsets.second);
  }
}

TEST(AddedVocabularyTest, ExtractAndNormalize) {
  std::unique_ptr<AddedVocabulary> added_vocabulary =
      get_added_vocabulary_from_string(
          "[{\"id\":0,\"content\":\"[PAD]\",\"single_word\":"
          "false,\"lstrip\":false,\"rstrip\":false,\"normalized\":false,"
          "\"special\":true},{\"id\":100,\"content\":\"[UNK]\",\"single_word\":"
          "false,\"lstrip\":false,\"rstrip\":false,\"normalized\":false,"
          "\"special\":true},{\"id\":101,\"content\":\"[CLS]\",\"single_word\":"
          "false,\"lstrip\":false,\"rstrip\":false,\"normalized\":false,"
          "\"special\":true},{\"id\":102,\"content\":\"[SEP]\",\"single_word\":"
          "false,\"lstrip\":false,\"rstrip\":false,\"normalized\":false,"
          "\"special\":true},{\"id\":103,\"content\":\"[MASK]\",\"single_"
          "word\":false,\"lstrip\":false,\"rstrip\":false,\"normalized\":false,"
          "\"special\":true}]");
  EXPECT_NE(added_vocabulary, nullptr);
  std::vector<Split> expected = {
      Split("[CLS]", {0, 5}), Split(" my name is, SLIM SHADY? ", {5, 30}),
      Split("[MASK]", {30, 36}), Split(" is my name!", {36, 48})};
  std::unique_ptr<Model> model = std::make_unique<WordPiece>(WordPiece(
      std::unordered_map<std::string, int>{
          {"[PAD]", 0},
          {"[UNK]", 100},
          {"[CLS]", 101},
          {"[SEP]", 102},
          {"[MASK]", 103},
      },
      "[UNK]", 100, "##"));
  EXPECT_NE(model, nullptr);
  added_vocabulary->add_tokens(added_vocabulary->added_tokens, model.get(),
                               nullptr);
  auto got = added_vocabulary->extract_and_normalize(
      nullptr, L"[CLS] my name is, SLIM SHADY? [MASK] is my name!");
  validate_added_vocabulary_splits(expected, got.splits);
}
