#include <torch/script.h>
#include <torch/torch.h>

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "tokenizers/common.h"
#include "tokenizers/tokenizer.h"

int main(int argc, char* argv[]) {
  torch::jit::script::Module module;
  try {
    module = torch::jit::load(std::string(argv[1]));
  } catch (const c10::Error& e) {
    std::cerr << "Error loading the model" << std::endl;
    return -1;
  }

  std::cout << "loaded the model from " << std::string(argv[1]) << std::endl;

  auto tokenizer = Tokenizer(std::string(argv[2]));
  auto encoding =
      tokenizer.encode(convert_from_string(std::string(argv[3])), true);
  int masked_index = -1;
  for (int i = 0; i < encoding.ids.size(); i++) {
    if (encoding.ids[i] == 103) {
      masked_index = i;
      break;
    }
  }
  torch::Tensor input_ids_tensor = torch::from_blob(
      encoding.ids.data(), {1, static_cast<long>(encoding.ids.size())},
      torch::kInt32);
  torch::Tensor attention_mask_tensor = torch::from_blob(
      encoding.attention_mask.data(),
      {1, static_cast<long>(encoding.attention_mask.size())}, torch::kInt32);
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input_ids_tensor);
  inputs.push_back(attention_mask_tensor);

  auto outputs = module.forward(inputs).toTuple();
  torch::Tensor predictions = outputs->elements()[0].toTensor();
  torch::Tensor prediction_for_masked_index = predictions[0][masked_index];
  auto predicted_index = prediction_for_masked_index.argmax().item<int32_t>();
  std::cout << tokenizer.decode({predicted_index}) << std::endl;

  return 0;
}
