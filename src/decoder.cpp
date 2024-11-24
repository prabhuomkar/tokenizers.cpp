// Copyright 2024 Omkar Prabhu
#include "tokenizers/decoder.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simdjson.h"
#include "tokenizers/common.h"

DECODER get_decoder(std::string type) {
  static const std::unordered_map<std::string, DECODER> types = {
      {"Sequence", SEQUENCE_DECODER},
      {"BPEDecoder", BPE_DECODER},
      {"ByteLevel", BYTE_LEVEL_DECODER},
      {"CTC", CTC_DECODER},
      {"Strip", STRIP_DECODER},
      {"Metaspace", METASPACE_DECODER},
      {"WordPiece", WORD_PIECE_DECODER},
      {"ByteFallback", BYTE_FALLBACK_DECODER},
      {"Replace", REPLACE_DECODER},
      {"Fuse", FUSE_DECODER}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_DECODER;
}

std::unique_ptr<Decoder> with_decoder(
    simdjson::ondemand::object decoder_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(decoder_params["type"].get_string()));
  if (get_decoder(type) == SEQUENCE_DECODER) {
    simdjson::ondemand::array seq_decoder_params_list =
        decoder_params["decoders"].get_array();
    std::vector<std::unique_ptr<Decoder>> seq_decoders;
    for (simdjson::ondemand::value decoder_val : seq_decoder_params_list) {
      simdjson::ondemand::object seq_decoder_params = decoder_val.get_object();
      std::unique_ptr<Decoder> seq_decoder = with_decoder(seq_decoder_params);
      if (seq_decoder != nullptr) {
        seq_decoders.push_back(std::move(seq_decoder));
      }
    }
    return std::make_unique<SequenceDecoder>(
        SequenceDecoder(std::move(seq_decoders)));
  } else if (get_decoder(type) == WORD_PIECE_DECODER) {
    val = decoder_params["prefix"].value();
    std::string prefix =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(val.get_string()));
    val = decoder_params["cleanup"].value();
    bool cleanup = val.type() == simdjson::ondemand::json_type::null
                       ? false
                       : static_cast<bool>(val.get_bool());
    return std::make_unique<WordPieceDecoder>(
        WordPieceDecoder(prefix, cleanup));
  } else if (get_decoder(type) == STRIP_DECODER) {
    val = decoder_params["content"].value();
    std::string content =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(val.get_string()));
    val = decoder_params["start"].value();
    int start = val.type() == simdjson::ondemand::json_type::null
                    ? 0
                    : static_cast<int>(val.get_int64());
    val = decoder_params["stop"].value();
    int stop = val.type() == simdjson::ondemand::json_type::null
                   ? 0
                   : static_cast<int>(val.get_int64());
    return std::make_unique<StripDecoder>(StripDecoder(content, start, stop));
  } else if (get_decoder(type) == REPLACE_DECODER) {
    val = decoder_params["pattern"].value();
    std::string pattern =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(
                              val["String"].value().get_string()));
    val = decoder_params["content"].value();
    std::string content =
        std::string(val.type() == simdjson::ondemand::json_type::null
                        ? ""
                        : static_cast<std::string_view>(val.get_string()));
    return std::make_unique<ReplaceDecoder>(ReplaceDecoder(pattern, content));
  } else if (get_decoder(type) == BYTE_FALLBACK_DECODER) {
    return std::make_unique<ByteFallbackDecoder>(ByteFallbackDecoder());
  } else if (get_decoder(type) == FUSE_DECODER) {
    return std::make_unique<FuseDecoder>(FuseDecoder());
  } else if (get_decoder(type) == BYTE_LEVEL_DECODER) {
    return std::make_unique<ByteLevelDecoder>(ByteLevelDecoder());
  }
  return nullptr;
}

WordPieceDecoder::WordPieceDecoder(std::string prefix, bool cleanup)
    : prefix(prefix), cleanup(cleanup) {}

std::string replace(std::string input, std::string from, std::string to) {
  int start = 0;
  while ((start = input.find(from, start)) != std::string::npos) {
    input.replace(start, from.length(), to);
    start += to.length();
  }
  return input;
}

std::string cleanup_token(std::string dirty_input) {
  dirty_input = replace(dirty_input, " .", ".");
  dirty_input = replace(dirty_input, " ?", "?");
  dirty_input = replace(dirty_input, " !", "!");
  dirty_input = replace(dirty_input, " ,", ",");
  dirty_input = replace(dirty_input, " ' ", "'");
  dirty_input = replace(dirty_input, " n't", "n't");
  dirty_input = replace(dirty_input, " 'm", "'m");
  dirty_input = replace(dirty_input, " do not", " don't");
  dirty_input = replace(dirty_input, " 's", "'s");
  dirty_input = replace(dirty_input, " 've", "'ve");
  dirty_input = replace(dirty_input, " 're", "'re");
  return dirty_input;
}

std::vector<std::string> WordPieceDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  std::vector<std::string> result;
  for (int i = 0; i < tokens.size(); i++) {
    std::string token = tokens[i];
    if (i != 0) {
      if (token.find(prefix) == 0) {
        token.replace(0, prefix.length(), "");
      } else {
        token = " " + token;
      }
    }
    if (cleanup) {
      token = cleanup_token(token);
    }
    result.push_back(token);
  }
  return result;
}

ReplaceDecoder::ReplaceDecoder(const std::string& pattern,
                               const std::string& content)
    : pattern(pattern), content(content) {}

std::vector<std::string> ReplaceDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  return {};
}

ByteFallbackDecoder::ByteFallbackDecoder() {}

std::vector<std::string> ByteFallbackDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  return {};
}

FuseDecoder::FuseDecoder() {}

std::vector<std::string> FuseDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  return {};
}

StripDecoder::StripDecoder(const std::string& content, int start, int stop)
    : content(content), start(start), stop(stop) {}

std::vector<std::string> StripDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  return {};
}

SequenceDecoder::SequenceDecoder(std::vector<std::unique_ptr<Decoder>> decoders)
    : decoders(std::move(decoders)) {}

std::vector<std::string> SequenceDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  for (const std::unique_ptr<Decoder>& decoder : decoders) {
    tokens = decoder->decode_chain(tokens);
  }
  return tokens;
}

ByteLevelDecoder::ByteLevelDecoder() {
  auto BYTES_CHAR = bytes_char();
  for (auto elem : BYTES_CHAR) {
    CHAR_BYTES[elem.second] = elem.first;
  }
}

std::vector<std::string> ByteLevelDecoder::decode_chain(
    std::vector<std::string> tokens) const {
  std::string result;
  for (const auto& token : tokens) {
    icu::UnicodeString unicode_token = icu::UnicodeString::fromUTF8(token);
    for (int i = 0; i < unicode_token.length(); i++) {
      auto ch = unicode_token.char32At(i);
      auto it = CHAR_BYTES.find(std::string(1, ch));
      if (it != CHAR_BYTES.end()) {
        result.push_back(static_cast<char>(it->second));
      } else {
        result.push_back(ch);
      }
    }
  }

  return {result};
}
