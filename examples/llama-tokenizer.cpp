#include "tokenizers.cpp/tokenizer.h"

int main() {
  auto tokenizer = Tokenizer("../../data/hf-internal-testing/llama-tokenizer");
  tokenizer.encode("A single sequence");
  return 0;
}
