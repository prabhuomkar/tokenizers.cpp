#include "tokenizers.cpp/tokenizer.h"

int main() {
  auto tokenizer = Tokenizer("data/bert-base-uncased");
  tokenizer.encode("A single sequence");
  return 0;
}
