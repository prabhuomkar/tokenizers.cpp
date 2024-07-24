import os
import sys
from tokenizers import Tokenizer

NAME = sys.argv[1]

tokenizer = Tokenizer.from_pretrained(NAME)
os.makedirs(f'../data/{NAME}', exist_ok=True)
tokenizer.save(path=f'../data/{NAME}/tokenizer.json')
