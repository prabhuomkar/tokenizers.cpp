import sys
from transformers import AutoTokenizer


NAME = sys.argv[1]

tokenizer = AutoTokenizer.from_pretrained(NAME)
tokenizer.save_pretrained(f"data/{NAME}")
print(tokenizer.encode_plus('A single sequence', return_offsets_mapping=True))
