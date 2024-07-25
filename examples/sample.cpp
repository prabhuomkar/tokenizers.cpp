// Copyright 2024 Omkar Prabhu
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "tokenizers/common.h"
#include "tokenizers/tokenizer.h"

using var_type =
    std::variant<int, std::optional<int>, std::string, std::pair<int, int>>;

void print_result(std::vector<var_type> elems) {
  std::cout << "[";
  for (int i = 0; i < elems.size(); i++) {
    auto elem = elems[i];
    std::visit(
        [](auto arg) {
          if constexpr (std::is_same_v<decltype(arg), int>) {
            std::cout << arg;
          } else if constexpr (std::is_same_v<decltype(arg),
                                              std::optional<int>>) {
            if (arg.has_value()) {
              std::cout << arg.value();
            } else {
              std::cout << "None";
            }
          } else if constexpr (std::is_same_v<decltype(arg), std::string>) {
            std::cout << arg;
          } else if constexpr (std::is_same_v<decltype(arg),
                                              std::pair<int, int>>) {
            std::cout << "(" << arg.first << ", " << arg.second << ")";
          }
        },
        elem);
    if (i != elems.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
}

int main(int argc, char* argv[]) {
  auto tokenizer = Tokenizer(std::string(argv[1]));
  std::wstring input = L"J'aime les crêpes au chocolat pour le petit-déjeuner.";
  auto result = tokenizer.encode(input);
  std::cout << "Encoding: " << convert_to_string(input) << std::endl;
  std::cout << "ids: ";
  print_result(std::vector<var_type>(result.ids.begin(), result.ids.end()));
  std::cout << "type_ids: ";
  print_result(
      std::vector<var_type>(result.type_ids.begin(), result.type_ids.end()));
  std::cout << "tokens: ";
  print_result(
      std::vector<var_type>(result.tokens.begin(), result.tokens.end()));
  std::cout << "words: ";
  print_result(std::vector<var_type>(result.words.begin(), result.words.end()));
  std::cout << "offsets: ";
  print_result(
      std::vector<var_type>(result.offsets.begin(), result.offsets.end()));
  std::cout << "attention_mask: ";
  print_result(std::vector<var_type>(result.attention_mask.begin(),
                                     result.attention_mask.end()));

  std::cout << "---------------------------------------------" << std::endl;
  std::string decoded_result = tokenizer.decode(result.ids);

  std::cout << "Decoding: ";
  print_result(std::vector<var_type>(result.ids.begin(), result.ids.end()));
  std::cout << "tokens: " << decoded_result << std::endl;
  return 0;
}
