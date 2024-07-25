import sys
from tokenizers import Tokenizer

input = "J'aime les crêpes au chocolat pour le petit-déjeuner."
name = sys.argv[1]

tokenizer = Tokenizer.from_file(name+"/tokenizer.json")
print(tokenizer.normalizer.normalize_str(input))
print("Encoding:", input)
result = tokenizer.encode(input)
print("ids:", result.ids)
print("type_ids:", result.type_ids)
print("tokens:", result.tokens)
print("words:", result.word_ids)
print("offsets:", result.offsets)
print("attention_mask:", result.attention_mask)
print("Decoding:", result.ids)
result = tokenizer.decode(result.ids)
print("tokens:", result)