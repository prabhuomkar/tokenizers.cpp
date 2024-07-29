from tokenizers import Tokenizer
from transformers import BertForMaskedLM
import torch


MODEL_NAME = "bert-base-uncased"
tokenizer = Tokenizer.from_pretrained(MODEL_NAME)
model = BertForMaskedLM.from_pretrained(MODEL_NAME)

text = "Mumbai is a major city in [MASK]."
encoding = tokenizer.encode(text, add_special_tokens=True)
masked_index = encoding.ids.index(103)
input_ids = torch.tensor(encoding.ids, dtype=torch.int32).unsqueeze(0)
attention_mask = torch.tensor(encoding.attention_mask, dtype=torch.int32).unsqueeze(0)


model = BertForMaskedLM.from_pretrained(MODEL_NAME, torchscript=True)

traced_model = torch.jit.trace(model, [input_ids, attention_mask])
torch.jit.save(traced_model, f"data/{MODEL_NAME}.pt")

loaded_model = torch.jit.load(f"data/{MODEL_NAME}.pt")
outputs = loaded_model(input_ids, attention_mask)
predictions = outputs[0]
predicted_index = torch.argmax(predictions[0, masked_index]).item()
predicted_token = tokenizer.decode([predicted_index])
print(predicted_token)

tokenizer.save("data/tokenizer.json")
