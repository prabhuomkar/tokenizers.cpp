import sys
from tokenizers import Tokenizer

input = "我喜欢学习中文。\u007fAçúcar é doce."
name = sys.argv[1]

tokenizer = Tokenizer.from_file(name+"/tokenizer.json")
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