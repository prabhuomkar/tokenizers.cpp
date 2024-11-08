# tokenizers.cpp Examples

- [Run](#run) - Simple tokenization only
- [LibTorch-BERT](#libtorch-bert) - Tokenization for PyTorch language model

## Run
Initialize and run a tokenizer using JSON configuration.

- Generate tokenizer config:
```
cd scripts
pip3 install -r requirements.txt
python3 generate_tokenizer_files.py <model-id> 
python3 generate_tokenizer_files.py bert-base-uncased
```
- Build and run the example:
```
mkdir build && cd build
cmake -DBUILD_EXAMPLES=ON .. && make
./examples/run/run ../data/bert-base-uncased 'Mumbai is a major city in [MASK].'
```

## LibTorch-BERT
Initialize and run a HuggingFace tokenizer using JSON config and language model.

- Generate tokenizer config:
```
cd scripts
pip3 install -r requirements.txt
python3 generate_tokenizer_files.py <model-id> 
python3 generate_tokenizer_files.py bert-base-uncased
```
- Convert PyTorch model to TorchScript module:
```
cd examples/libtorch-bert
pip3 install -r requirements.txt
python3 generate_torchscript.py
```
- Build and run the example:
```
mkdir build && cd build
cmake -DBUILD_EXAMPLES=ON .. && make
./examples/libtorch-bert/libtorch-bert ../data/mobilebert-uncased.pt ../data/bert-base-uncased 'Mumbai is a major city in [MASK].'
```
