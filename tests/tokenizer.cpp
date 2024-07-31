// Copyright 2024 Omkar Prabhu
#include "tokenizers/tokenizer.h"

#include <gtest/gtest.h>

#include <memory>
#include <typeinfo>

void assert_tokenizer_encoding(Encoding expected, Encoding got) {
  EXPECT_EQ(expected.ids, got.ids);
  EXPECT_EQ(expected.offsets, got.offsets);
  EXPECT_EQ(expected.tokens, got.tokens);
  EXPECT_EQ(expected.type_ids, got.type_ids);
  EXPECT_EQ(expected.words, got.words);
  EXPECT_EQ(expected.special_tokens_mask, got.special_tokens_mask);
  EXPECT_EQ(expected.attention_mask, got.attention_mask);
  EXPECT_EQ(expected.overflowing.size(), got.overflowing.size());
  if (expected.overflowing.size() > 0) {
    for (int i = 0; i < expected.overflowing.size(); i++) {
      assert_tokenizer_encoding(expected.overflowing[i], got.overflowing[i]);
    }
  }
}

TEST(TokenizerTest, Success) {
  // empty components
  auto tokenizer =
      Tokenizer("",
                "{\"truncation\":null,\"padding\":null,\"added_"
                "tokens\":null,\"normalizer\":null,\"pre_tokenizer\":null,"
                "\"model\":null,\"post_processor\":null,\"decoder\":null}");
  Encoding expected = Encoding({}, {}, {}, {}, {}, {}, {});
  Encoding got = tokenizer.encode(L"Hello World!", true);
  assert_tokenizer_encoding(expected, got);
  // valid components
  tokenizer = Tokenizer(
      "",
      "{\"version\":\"1.0\",\"truncation\":null,\"padding\":null,\"added_"
      "tokens\":[{\"id\":0,\"content\":\"[PAD]\",\"single_word\":false,"
      "\"lstrip\":false,\"rstrip\":false,\"normalized\":false,\"special\":true}"
      ",{\"id\":100,\"content\":\"[UNK]\",\"single_word\":false,\"lstrip\":"
      "false,\"rstrip\":false,\"normalized\":false,\"special\":true},{\"id\":"
      "101,\"content\":\"[CLS]\",\"single_word\":false,\"lstrip\":false,"
      "\"rstrip\":false,\"normalized\":false,\"special\":true},{\"id\":102,"
      "\"content\":\"[SEP]\",\"single_word\":false,\"lstrip\":false,\"rstrip\":"
      "false,\"normalized\":false,\"special\":true},{\"id\":103,\"content\":\"["
      "MASK]\",\"single_word\":false,\"lstrip\":false,\"rstrip\":false,"
      "\"normalized\":false,\"special\":true}],\"normalizer\":{\"type\":"
      "\"BertNormalizer\",\"clean_text\":true,\"handle_chinese_chars\":true,"
      "\"strip_accents\":null,\"lowercase\":true},\"pre_tokenizer\":{\"type\":"
      "\"BertPreTokenizer\"},\"post_processor\":{\"type\":"
      "\"TemplateProcessing\",\"single\":[{\"SpecialToken\":{\"id\":\"[CLS]\","
      "\"type_id\":0}},{\"Sequence\":{\"id\":\"A\",\"type_id\":0}},{"
      "\"SpecialToken\":{\"id\":\"[SEP]\",\"type_id\":0}}],\"pair\":[{"
      "\"SpecialToken\":{\"id\":\"[CLS]\",\"type_id\":0}},{\"Sequence\":{"
      "\"id\":\"A\",\"type_id\":0}},{\"SpecialToken\":{\"id\":\"[SEP]\",\"type_"
      "id\":0}},{\"Sequence\":{\"id\":\"B\",\"type_id\":1}},{\"SpecialToken\":{"
      "\"id\":\"[SEP]\",\"type_id\":1}}],\"special_tokens\":{\"[CLS]\":{\"id\":"
      "\"[CLS]\",\"ids\":[101],\"tokens\":[\"[CLS]\"]},\"[SEP]\":{\"id\":\"["
      "SEP]\",\"ids\":[102],\"tokens\":[\"[SEP]\"]}}},\"decoder\":{\"type\":"
      "\"WordPiece\",\"prefix\":\"##\",\"cleanup\":true},\"model\":{\"type\":"
      "\"WordPiece\",\"unk_token\":\"[UNK]\",\"continuing_subword_prefix\":\"##"
      "\",\"max_input_chars_per_word\":100,\"vocab\":{\"[PAD]\":0}}}");
  expected = Encoding({101, 0, 0, 0, 102}, {0, 0, 0, 0, 0},
                      {"[CLS]", "[UNK]", "[UNK]", "[UNK]", "[SEP]"},
                      {std::nullopt, 0, 1, 2, std::nullopt},
                      {{0, 0}, {0, 5}, {6, 11}, {11, 12}, {0, 0}},
                      {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1});
  got = tokenizer.encode(L"Hello World!", true);
  assert_tokenizer_encoding(expected, got);
}

TEST(TokenizerTest, Error) {
  // empty arguments
  EXPECT_THROW({ auto tokenizer = Tokenizer("", ""); }, std::invalid_argument);
  // invalid json config file path
  EXPECT_THROW(
      { auto tokenizer = Tokenizer("invalid-file-path", ""); },
      std::runtime_error);
  // invalid json config string
  EXPECT_THROW({ auto tokenizer = Tokenizer("", "{}"); }, std::runtime_error);
}
