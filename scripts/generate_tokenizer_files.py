import sys
from transformers import AutoTokenizer


NAME = sys.argv[1]

tokenizer = AutoTokenizer.from_pretrained(NAME)
tokenizer.save_pretrained(f"data/{NAME}")
print(tokenizer.encode_plus('我喜欢学习中文。Açúcar é doce.', return_offsets_mapping=True))
