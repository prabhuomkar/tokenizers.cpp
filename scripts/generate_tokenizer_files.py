from transformers import AutoTokenizer


NAME = "bert-base-cased"

tokenizer = AutoTokenizer.from_pretrained(NAME)
tokenizer.save_pretrained(f"data/{NAME}")
