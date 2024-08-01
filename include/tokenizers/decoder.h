// Copyright 2024 Omkar Prabhu
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "simdjson.h"

enum DECODER {
  SEQUENCE_DECODER,
  BPE_DECODER,
  BYTE_LEVEL_DECODER,
  CTC_DECODER,
  METASPACE_DECODER,
  WORD_PIECE_DECODER,
  BYTE_FALLBACK_DECODER,
  FUSE_DECODER,
  STRIP_DECODER,
  REPLACE_DECODER,
  UNKNOWN_DECODER
};

DECODER get_decoder(std::string type);

class Decoder {
 public:
  virtual ~Decoder() = default;
  virtual std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const = 0;
};

std::unique_ptr<Decoder> with_decoder(
    simdjson::ondemand::object decoder_params);

class WordPieceDecoder : public Decoder {
 public:
  explicit WordPieceDecoder(std::string prefix = "##", bool cleanup = true);
  std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const override;

 private:
  std::string prefix;
  bool cleanup;
};

class ReplaceDecoder : public Decoder {
 public:
  explicit ReplaceDecoder(const std::string& pattern,
                          const std::string& content);
  std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const override;

 private:
  std::string pattern;
  std::string content;
};

class ByteFallbackDecoder : public Decoder {
 public:
  ByteFallbackDecoder();
  std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const override;
};

class FuseDecoder : public Decoder {
 public:
  FuseDecoder();
  std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const override;
};

class StripDecoder : public Decoder {
 public:
  explicit StripDecoder(const std::string& content, int start, int stop);
  std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const override;

 private:
  std::string content;
  int start;
  int stop;
};

class SequenceDecoder : public Decoder {
 public:
  explicit SequenceDecoder(std::vector<std::unique_ptr<Decoder>> decoders);
  std::vector<std::string> decode_chain(
      std::vector<std::string> tokens) const override;

 private:
  std::vector<std::unique_ptr<Decoder>> decoders;
};
