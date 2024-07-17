#include "tokenizers.cpp/tokenizer.h"

int main() {
  auto tokenizer = Tokenizer("../../data/hf-internal-testing/llama-tokenizer");
  auto result = tokenizer.encode(L"A single sequence");
  std::cout << "ids: " << "[";
  for (int i = 0; i < result.ids.size(); i++) {
    std::cout << result.ids[i];
    if (i != result.ids.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
  std::cout << "type_ids: " << "[";
  for (int i = 0; i < result.type_ids.size(); i++) {
    std::cout << result.type_ids[i];
    if (i != result.type_ids.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
  std::cout << "tokens: " << "[";
  for (int i = 0; i < result.tokens.size(); i++) {
    std::cout << result.tokens[i];
    if (i != result.tokens.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
  std::cout << "words: " << "[";
  for (int i = 0; i < result.words.size(); i++) {
    std::cout << result.words[i];
    if (i != result.words.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
  std::cout << "offsets: " << "[";
  for (int i = 0; i < result.offsets.size(); i++) {
    std::cout << "(" << result.offsets[i].first << ","
              << result.offsets[i].second << ")";
    if (i != result.offsets.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
  return 0;
}
