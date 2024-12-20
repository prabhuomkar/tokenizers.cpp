// Copyright 2024 Omkar Prabhu
#include "tokenizers/decoder.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "simdjson.h"

std::unique_ptr<Decoder> get_decoder_from_string(std::string json) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded_json(json);
  simdjson::ondemand::document doc = parser.iterate(json);
  simdjson::ondemand::object decoder_params = doc.get_object().value();
  return with_decoder(decoder_params);
}

TEST(WordPieceDecoderTest, Simple) {
  std::unique_ptr<Decoder> decoder = get_decoder_from_string(
      "{\"type\":\"WordPiece\",\"prefix\":\"##\",\"cleanup\":true}");
  EXPECT_NE(decoder, nullptr);
  std::vector<std::string> input = {"##uelo", "Ara", "##új",
                                    "##o",    "No",  "##guera"};
  std::vector<std::string> got = decoder->decode_chain(input);
  std::vector<std::string> expected = {"##uelo", " Ara", "új",
                                       "o",      " No",  "guera"};
  EXPECT_EQ(expected, got);
}

TEST(WordPieceDecoderTest, Cleanup) {
  std::unique_ptr<Decoder> decoder = get_decoder_from_string(
      "{\"type\":\"WordPiece\",\"prefix\":\"##\",\"cleanup\":true}");
  EXPECT_NE(decoder, nullptr);
  std::vector<std::string> input = {"##uelo", "Ara ?", "##új",
                                    "##o",    "No .",  "##guera"};
  std::vector<std::string> got = decoder->decode_chain(input);
  std::vector<std::string> expected = {"##uelo", " Ara?", "új",
                                       "o",      " No.",  "guera"};
  EXPECT_EQ(expected, got);
}

TEST(SequenceDecoderTest, Simple) {
  std::unique_ptr<Decoder> decoder = get_decoder_from_string(
      "{\"type\":\"Sequence\",\"decoders\":[{\"type\":\"WordPiece\",\"prefix\":"
      "\"##\",\"cleanup\":true}]}");
  EXPECT_NE(decoder, nullptr);
  std::vector<std::string> input = {"##uelo", "Ara", "##új",
                                    "##o",    "No",  "##guera"};
  std::vector<std::string> got = decoder->decode_chain(input);
  std::vector<std::string> expected = {"##uelo", " Ara", "új",
                                       "o",      " No",  "guera"};
  EXPECT_EQ(expected, got);
}

TEST(ByteLevelDecoderTest, Simple) {
  std::unique_ptr<Decoder> decoder =
      get_decoder_from_string("{\"type\":\"ByteLevel\"}");
  EXPECT_NE(decoder, nullptr);
  std::vector<std::string> input = {"How", "Ġare", "Ġya", "Ġdoing", "?"};
  std::vector<std::string> got = decoder->decode_chain(input);
  std::vector<std::string> expected = {"How are ya doing?"};
  EXPECT_EQ(expected, got);
}
