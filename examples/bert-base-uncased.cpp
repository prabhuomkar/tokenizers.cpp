#include "tokenizers.cpp/tokenizer.h"

int main() {
  auto tokenizer = Tokenizer("data/bert-base-uncased");
  tokenizer.encode("test", true, true);
  return 0;
}
