import sys
from tokenizers import Tokenizer

NAME = sys.argv[1]

input = '我喜欢学习中文。Açúcar é doce.'
tokenizer = Tokenizer.from_pretrained(NAME)
res = []
splits = tokenizer.pre_tokenizer.pre_tokenize_str(tokenizer.normalizer.normalize_str(input))
for tokens in [tokenizer.model.tokenize(x[0]) for x in splits]:
    for token in tokens:
        res.append((token.value, token.offsets))
print(res)
print('---------------------------')
output = tokenizer.encode(input)
print('ids:', output.ids)
print('tokens:', output.tokens)
print('offsets:', output.offsets)