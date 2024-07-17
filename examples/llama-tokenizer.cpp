#include "tokenizers.cpp/tokenizer.h"

int main() {
  auto tokenizer = Tokenizer("../../data/hf-internal-testing/llama-tokenizer");
  auto result = tokenizer.encode(L"A single sequence");
  return 0;
}
