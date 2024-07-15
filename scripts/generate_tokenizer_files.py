import sys
from transformers import AutoTokenizer


NAME = sys.argv[1]

tokenizer = AutoTokenizer.from_pretrained(NAME)
tokenizer.save_pretrained(f"data/{NAME}")
