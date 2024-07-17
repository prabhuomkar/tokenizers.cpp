#include "tokenizers.cpp/tokenizer.h"

int main() {
  auto tokenizer = Tokenizer("../../data/bert-base-uncased");
  auto result = tokenizer.encode(L"A single sequence");
  return 0;
}
