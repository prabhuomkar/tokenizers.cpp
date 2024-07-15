// Copyright 2024 Omkar Prabhu
#include "tokenizers.cpp/normalizer.h"

#include <gtest/gtest.h>

#include <memory>
#include <typeinfo>

#include "simdjson.h"

std::optional<std::unique_ptr<Normalizer>> get_normalizer_from_string(
    std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object normalizer_params = doc.get_object().value();
  return with_normalizer(normalizer_params);
}

TEST(SequenceNormalizerTest, Simple) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string(
          "{\"type\":\"Sequence\",\"normalizers\":[{\"type\":\"Prepend\","
          "\"prepend\":\"▁\"},{\"type\":\"Replace\",\"pattern\":{\"String\":\" "
          "\"},\"content\":\"▁\"}]}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got =
      normalizer.value()->normalize(L"Hello World!\tThis is a test.\n");
  EXPECT_EQ(L"▁Hello▁▁World!\tThis▁▁is▁▁a▁▁test.\n▁", got);
}

TEST(BertNormalizerTest, CleanText) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string(
          "{\"type\":\"BertNormalizer\",\"clean_text\":true,\"handle_chinese_"
          "chars\":false,\"strip_accents\":null,\"lowercase\":false}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got =
      normalizer.value()->normalize(L"Hello World!\tThis is a test.\n");
  EXPECT_EQ(L"Hello World! This is a test. ", got);
}

TEST(BertNormalizerTest, HandleChineseChars) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string(
          "{\"type\":\"BertNormalizer\",\"clean_text\":false,\"handle_chinese_"
          "chars\":true,\"strip_accents\":null,\"lowercase\":false}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got = normalizer.value()->normalize(L"Hello 世界!");
  EXPECT_EQ(L"Hello  世  界 !", got);
}

TEST(BertNormalizerTest, StripAccents) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string(
          "{\"type\":\"BertNormalizer\",\"clean_text\":false,\"handle_chinese_"
          "chars\":false,\"strip_accents\":true,\"lowercase\":false}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got =
      normalizer.value()->normalize(L"Hélló Wórld! Thïs ïs á tést.");
  EXPECT_EQ(L"Hello World! This is a test.", got);
}

TEST(BertNormalizerTest, Lowercase) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string(
          "{\"type\":\"BertNormalizer\",\"clean_text\":false,\"handle_chinese_"
          "chars\":false,\"strip_accents\":null,\"lowercase\":true}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got =
      normalizer.value()->normalize(L"HELLO WORLD! THIS IS A TEST!");
  EXPECT_EQ(L"hello world! this is a test!", got);
}

TEST(PrependNormalizerTest, Simple) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string("{\"type\":\"Prepend\",\"prepend\":\"_\"}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got = normalizer.value()->normalize(L"Hello World!");
  EXPECT_EQ(L"_Hello _World! ", got);
}

TEST(NFDNormalizerTest, Simple) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string("{\"type\":\"NFD\"}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got = normalizer.value()->normalize(L"Héllo World!");
  EXPECT_EQ(L"Héllo World!", got);
}

TEST(ReplaceNormalizerTest, ReplaceContent) {
  std::optional<std::unique_ptr<Normalizer>> normalizer =
      get_normalizer_from_string(
          "{\"type\":\"Replace\",\"pattern\":{\"String\":\" "
          "\"},\"content\":\"▁\"}");
  EXPECT_EQ(true, normalizer.has_value());
  std::wstring got = normalizer.value()->normalize(L"Hello World!");
  EXPECT_EQ(L"Hello▁World!", got);
}
