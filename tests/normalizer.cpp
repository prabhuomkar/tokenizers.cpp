// Copyright 2024 Omkar Prabhu
#include "tokenizers/normalizer.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <typeinfo>

#include "simdjson.h"

std::unique_ptr<Normalizer> get_normalizer_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object normalizer_params = doc.get_object().value();
  return with_normalizer(normalizer_params);
}

TEST(SequenceNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"Sequence\",\"normalizers\":[{\"type\":\"Prepend\","
      "\"prepend\":\"▁\"},{\"type\":\"Replace\",\"pattern\":{\"String\":\" "
      "\"},\"content\":\"▁\"}]}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(
      NormalizedString(L"Hello World!\tThis is a test.\n"));
  EXPECT_EQ(L"▁Hello▁World!\tThis▁is▁a▁test.\n", normalized.normalized);
}

TEST(BertNormalizerTest, CleanText) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"BertNormalizer\",\"clean_text\":true,\"handle_chinese_"
      "chars\":false,\"strip_accents\":null,\"lowercase\":false}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(
      NormalizedString(L"Hello World!\tThis is a test.\n"));
  EXPECT_EQ(L"Hello World! This is a test. ", normalized.normalized);
}

TEST(BertNormalizerTest, HandleChineseChars) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"BertNormalizer\",\"clean_text\":false,\"handle_chinese_"
      "chars\":true,\"strip_accents\":null,\"lowercase\":false}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"Hello 世界!"));
  EXPECT_EQ(L"Hello  世  界 !", normalized.normalized);
}

TEST(BertNormalizerTest, StripAccents) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"BertNormalizer\",\"clean_text\":false,\"handle_chinese_"
      "chars\":false,\"strip_accents\":true,\"lowercase\":false}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized =
      normalizer->normalize(NormalizedString(L"Hélló Wórld! Thïs ïs á tést."));
  EXPECT_EQ(L"Hello World! This is a test.", normalized.normalized);
}

TEST(BertNormalizerTest, Lowercase) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"BertNormalizer\",\"clean_text\":false,\"handle_chinese_"
      "chars\":false,\"strip_accents\":null,\"lowercase\":true}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized =
      normalizer->normalize(NormalizedString(L"HELLO WORLD! THIS IS A TEST!"));
  EXPECT_EQ(L"hello world! this is a test!", normalized.normalized);
}

TEST(PrependNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"Prepend\",\"prepend\":\"_\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"Hello World!"));
  EXPECT_EQ(L"_Hello World!", normalized.normalized);
}

TEST(NFCNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFC\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"a\u0301"));
  EXPECT_EQ(L"\u00E1", normalized.normalized);
}

TEST(NFCNormalizerTest, Error) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFC\"}");
  EXPECT_NE(normalizer, nullptr);
  EXPECT_THROW(
      {
        auto normalized = normalizer->normalize(
            NormalizedString(convert_from_string("invalid\xFF")));
      },
      std::runtime_error);
}

TEST(NFDNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFD\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"Héllo World!"));
  EXPECT_EQ(L"Héllo World!", normalized.normalized);
}

TEST(NFDNormalizerTest, Error) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFD\"}");
  EXPECT_NE(normalizer, nullptr);
  EXPECT_THROW(
      {
        auto normalized = normalizer->normalize(
            NormalizedString(convert_from_string("invalid\xFF")));
      },
      std::runtime_error);
}

TEST(NFKCNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFKC\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"\u{fb01}"));
  EXPECT_EQ(L"fi", normalized.normalized);
}

TEST(NFKCNormalizerTest, Error) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFKC\"}");
  EXPECT_NE(normalizer, nullptr);
  EXPECT_THROW(
      {
        auto normalized = normalizer->normalize(
            NormalizedString(convert_from_string("invalid\xFF")));
      },
      std::runtime_error);
}

TEST(NFKDNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFKD\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"Héllo World!"));
  EXPECT_EQ(L"Héllo World!", normalized.normalized);
}

TEST(NFKDNormalizerTest, Error) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"NFKD\"}");
  EXPECT_NE(normalizer, nullptr);
  EXPECT_THROW(
      {
        auto normalized = normalizer->normalize(
            NormalizedString(convert_from_string("invalid\xFF")));
      },
      std::runtime_error);
}

TEST(ReplaceNormalizerTest, ReplaceContent) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"Replace\",\"pattern\":{\"String\":\" "
      "\"},\"content\":\"▁\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized = normalizer->normalize(NormalizedString(L"Hello World!"));
  EXPECT_EQ(L"Hello▁World!", normalized.normalized);
}

TEST(StripNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer = get_normalizer_from_string(
      "{\"type\":\"Strip\",\"strip_left\":true,\"strip_right\":true}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized =
      normalizer->normalize(NormalizedString(L"  Hello World!  "));
  EXPECT_EQ(L"Hello World!", normalized.normalized);
}

TEST(StripAccentsNormalizerTest, Simple) {
  std::unique_ptr<Normalizer> normalizer =
      get_normalizer_from_string("{\"type\":\"StripAccents\"}");
  EXPECT_NE(normalizer, nullptr);
  auto normalized =
      normalizer->normalize(NormalizedString(L"e\u{304}\u{304}\u{304}o"));
  EXPECT_EQ(L"eo", normalized.normalized);
}
