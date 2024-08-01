// Copyright 2024 Omkar Prabhu
#include "tokenizers/model.h"

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "simdjson.h"
#include "tokenizers/common.h"
#include "tokenizers/utils.h"

MODEL get_model(std::string type) {
  static const std::unordered_map<std::string, MODEL> types = {
      {"BPE", BPE_MODEL},
      {"Unigram", UNIGRAM_MODEL},
      {"WordLevel", WORD_LEVEL_MODEL},
      {"WordPiece", WORD_PIECE_MODEL}};

  auto it = types.find(type);
  if (it != types.end()) {
    return it->second;
  }
  return UNKNOWN_MODEL;
}

std::unique_ptr<Model> with_model(simdjson::ondemand::object model_params) {
  simdjson::ondemand::value val;
  std::string type = std::string(
      static_cast<std::string_view>(model_params["type"].get_string()));
  val = model_params["vocab"].value();
  std::unordered_map<std::string, int> vocab =
      val.type() == simdjson::ondemand::json_type::null
          ? std::unordered_map<std::string, int>()
          : get_map_ints_from_json(val.get_object());
  if (get_model(type) == WORD_PIECE_MODEL) {
    val = model_params["unk_token"].value();
    std::string unk_token =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["max_input_chars_per_word"].value();
    int max_input_chars_per_word =
        val.type() == simdjson::ondemand::json_type::null
            ? 0
            : static_cast<int>(val.get_int64());
    val = model_params["continuing_subword_prefix"].value();
    std::string continuing_subword_prefix =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    return std::make_unique<WordPiece>(WordPiece(
        vocab, unk_token, max_input_chars_per_word, continuing_subword_prefix));
  } else if (get_model(type) == BPE_MODEL) {
    val = model_params["merges"].value();
    std::vector<std::string> merges;
    if (val.type() != simdjson::ondemand::json_type::null) {
      simdjson::ondemand::array merges_array = val.get_array();
      for (auto element : merges_array) {
        merges.push_back(
            std::string(static_cast<std::string_view>(element.get_string())));
      }
    }
    val = model_params["dropout"].value();
    float dropout = val.type() == simdjson::ondemand::json_type::null
                        ? 0.0
                        : static_cast<float>(val.get_double());
    val = model_params["unk_token"].value();
    std::string unk_token =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["continuing_subword_prefix"].value();
    std::string continuing_subword_prefix =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["end_of_word_suffix"].value();
    std::string end_of_word_suffix =
        val.type() == simdjson::ondemand::json_type::null
            ? ""
            : std::string(static_cast<std::string_view>(val.get_string()));
    val = model_params["fuse_unk"].value();
    bool fuse_unk = val.type() == simdjson::ondemand::json_type::null
                        ? false
                        : static_cast<bool>(val.get_bool());
    val = model_params["byte_fallback"].value();
    bool byte_fallback = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());
    val = model_params["ignore_merges"].value();
    bool ignore_merges = val.type() == simdjson::ondemand::json_type::null
                             ? false
                             : static_cast<bool>(val.get_bool());
    return std::make_unique<BPE>(
        BPE(vocab, merges, dropout, unk_token, continuing_subword_prefix,
            end_of_word_suffix, fuse_unk, byte_fallback, ignore_merges));
  }
  return nullptr;
}

Model::Model(const std::unordered_map<std::string, int>& vocab) : vocab(vocab) {
  for (auto pair : vocab) {
    vocab_r[pair.second] = pair.first;
  }
}

int Model::get_vocab_size() const { return vocab.size(); }

std::optional<int> Model::token_to_id(std::string token) {
  auto it = vocab.find(token);
  if (it != vocab.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::optional<std::string> Model::id_to_token(int id) {
  auto it = vocab_r.find(id);
  if (it != vocab_r.end()) {
    return it->second;
  }
  return std::nullopt;
}

WordPiece::WordPiece(const std::unordered_map<std::string, int>& vocab,
                     const std::string& unk_token, int max_input_chars_per_word,
                     const std::string& continuing_subword_prefix)
    : Model(vocab),
      unk_token(unk_token),
      max_input_chars_per_word(max_input_chars_per_word),
      continuing_subword_prefix(continuing_subword_prefix) {}

PreTokenizedString WordPiece::tokenize(PreTokenizedString pre_tokenized) const {
  for (auto& split : pre_tokenized.splits) {
    icu::UnicodeString unicode_sequence =
        icu::UnicodeString::fromUTF8(split.normalized);
    int char_len = unicode_sequence.length();
    if (char_len > max_input_chars_per_word) {
      auto it = vocab.find(unk_token);
      if (it == vocab.end()) {
        split.tokens = {Token(0, unk_token, {0, char_len})};
        continue;
      }
      split.tokens = {Token(it->second, unk_token, {0, char_len})};
      continue;
    }
    bool is_bad = false;
    int start = 0;
    std::vector<Token> sub_tokens;
    while (start < unicode_sequence.length()) {
      int end = unicode_sequence.length();
      std::optional<Token> cur_sequence_token = std::nullopt;
      while (start < end) {
        std::string sub_sequence;
        icu::UnicodeString unicode_sub_sequence =
            unicode_sequence.tempSubString(start, end - start);
        unicode_sub_sequence.toUTF8String(sub_sequence);
        if (start > 0) {
          sub_sequence = continuing_subword_prefix + sub_sequence;
        }
        if (vocab.count(sub_sequence) != 0) {
          auto it = vocab.find(sub_sequence);
          if (it != vocab.end()) {
            cur_sequence_token = Token(it->second, sub_sequence, {start, end});
          }
          break;
        }
        end -= 1;
      }
      if (!cur_sequence_token.has_value()) {
        is_bad = true;
        break;
      }
      sub_tokens.push_back(cur_sequence_token.value());
      start = end;
    }
    if (is_bad) {
      auto it = vocab.find(unk_token);
      if (it == vocab.end()) {
        split.tokens = {Token(0, unk_token, {0, char_len})};
        continue;
      }
      split.tokens = {Token(it->second, unk_token, {0, char_len})};
      continue;
    }
    split.tokens = sub_tokens;
  }
  return pre_tokenized;
}

BPE::BPE(const std::unordered_map<std::string, int>& vocab,
         std::vector<std::string> merges, float dropout,
         const std::string& unk_token,
         const std::string& continuing_subword_prefix,
         const std::string& end_of_word_suffix, bool fuse_unk,
         bool byte_fallback, bool ignore_merges)
    : Model(vocab),
      merges(merges),
      dropout(dropout),
      unk_token(unk_token),
      continuing_subword_prefix(continuing_subword_prefix),
      end_of_word_suffix(end_of_word_suffix),
      fuse_unk(fuse_unk),
      byte_fallback(byte_fallback),
      ignore_merges(ignore_merges) {}

void BPE::merge_word(const std::string& word) {}

std::vector<Token> BPE::word_to_tokens() { return {}; }

std::vector<Token> BPE::tokenize_with_cache(const std::string& sequence) {
  return {};
}

PreTokenizedString BPE::tokenize(PreTokenizedString pre_tokenized) const {
  for (auto& split : pre_tokenized.splits) {
    std::string sequence = split.normalized;
    if (dropout == 0.0f) {
      split.tokens = tokenize_with_cache(sequence);
    } else {
      merge_word(sequence);
      split.tokens = word_to_tokens();
    }
  }
  return pre_tokenized;
}
