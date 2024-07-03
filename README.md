# tokenizers.cpp
Tokenizers in C++

## Notes
```cpp
auto tokenizer = tokenizers::from_file(string path);

std::vector<tokenizers::Encoding> tokenizer.encode(string sequence, bool is_pretokenized, bool add_special_tokens);

std::string result = tokenizer.decode(std::vector<int> ids, bool skip_special_tokens);
```