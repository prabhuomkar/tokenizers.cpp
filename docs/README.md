# tokenizers.cpp Documentation

## Goals
- Not intended to be a tutorial/course of Huggingface Tokenizers
- Build a portable C++ version of Huggingface Tokenizers which can run with PyTorch's LibTorch 
C++ API and other platforms like Android and iOS
- Demonstrate an example of running a traced/scripted HuggingFace model with tokenizer and 
run in a C++ environment without any Python dependencies
- Run some benchmarks to compare performance with Rust version and improve C++ version

## Support Matrix
This section will provide information about the [Tokenizer components](https://huggingface.co/docs/tokenizers/components) and their respective support status.  
- ✅ - Done
- 🚧 - Planned/Work In Progress

### Normalizer
| **Name** | [BertNormalizer] | [StripNormalizer] | [StripAccents] | [NFC] | [NFD] | [NFKC] | [NFKD] | [SequenceNormalizer] | [Lowercase] | [Nmt] | [Precompiled] | [Replace] | [Prepend] | [ByteLevelNormalizer] | 
| - | - | - | - | - | - | - | - | - | - | - | - | - | - | - |
| **Status** | ✅ | 🚧 | 🚧 | 🚧 | ✅ | 🚧 | 🚧 | ✅ | 🚧 | 🚧 | 🚧 | ✅ | ✅ | 🚧 | 

### PreTokenizer
| **Name** | [BertPreTokenizer] | [ByteLevelPreTokenizer] | [CharDelimiterSplit] | [Metaspace] | [Whitespace] | [SequencePreTokenizer] | [Split] | [Punctuation] | [WhitespaceSplit] | [Digits] | [UnicodeScripts] |  
| - | - | - | - | - | - | - | - | - | - | - | - | 
| **Status** | ✅ | ✅ | 🚧 | 🚧 | 🚧 | ✅ | ✅ | 🚧 | 🚧 | 🚧 | 🚧 |

### Model
| **Name** | [BPE] | [WordPiece] | [WordLevel] | [Unigram] |
| - | - | - | - | - |
| **Status** | 🚧 | ✅ | 🚧 | 🚧 |

### PostProcessor
| **Name** | [RobertaProcessing] | [BertProcessing] | [ByteLevelProcessing] | [TemplateProcessing] | [SequenceProcessing] | 
| - | - | - | - | - | - |
| **Status** | 🚧 | 🚧 | ✅ | ✅ | ✅ |

### Decoder
| **Name** | [BPEDecoder] | [ByteLevelDecoder] | [WordPieceDecoder] | [MetaspaceDecoder] | [CTC] | [SequenceDecoder] | [ReplaceDecoder] | [Fuse] | [StripDecoder] | [ByteFallbackDecoder] |
| - | - | - | - | - | - | - | - | - | - | - |
| **Status** | 🚧 | ✅ | ✅ | 🚧 | 🚧 | ✅ | ✅ | ✅ | ✅ | ✅ |

<!-- Normalizers -->
[BertNormalizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/bert.rs
[StripNormalizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/strip.rs
[StripAccents]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/strip.rs
[NFC]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/unicode.rs
[NFD]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/unicode.rs
[NFKC]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/unicode.rs
[NFKD]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/unicode.rs
[SequenceNormalizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/utils.rs
[Lowercase]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/utils.rs
[Nmt]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/unicode.rs
[Precompiled]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/precompiled.rs
[Replace]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/replace.rs
[Prepend]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/prepend.rs
[ByteLevelNormalizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/byte_level.rs

<!-- PreTokenizers -->
[BertPreTokenizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/bert.rs
[ByteLevelPreTokenizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/byte_level.rs
[CharDelimiterSplit]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/delimiter.rs
[Metaspace]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/metaspace.rs
[Whitespace]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/whitespace.rs
[SequencePreTokenizer]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/sequence.rs
[Split]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/split.rs
[Punctuation]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/punctuation.rs
[WhitespaceSplit]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/whitespace.rs
[Digits]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/digits.rs
[UnicodeScripts]: https://github.com/huggingface/tokenizers/tree/main/tokenizers/src/pre_tokenizers/unicode_scripts

<!-- Model -->
[BPE]: https://github.com/huggingface/tokenizers/tree/main/tokenizers/src/models/bpe
[WordPiece]: https://github.com/huggingface/tokenizers/tree/main/tokenizers/src/models/wordpiece
[WordLevel]: https://github.com/huggingface/tokenizers/tree/main/tokenizers/src/models/wordlevel
[Unigram]: https://github.com/huggingface/tokenizers/tree/main/tokenizers/src/models/unigram

<!-- PostProcessors -->
[RobertaProcessing]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/processors/roberta.rs
[BertProcessing]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/processors/bert.rs
[ByteLevelProcessing]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/byte_level.rs
[TemplateProcessing]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/processors/template.rs
[SequenceProcessing]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/processors/sequence.rs

<!-- Decoders -->
[BPEDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/bpe.rs
[ByteLevelDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/byte_level.rs
[WordPieceDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/wordpiece.rs
[MetaspaceDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/pre_tokenizers/metaspace.rs
[CTC]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/ctc.rs
[SequenceDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/sequence.rs
[ReplaceDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/normalizers/replace.rs
[Fuse]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/fuse.rs
[StripDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/strip.rs
[ByteFallbackDecoder]: https://github.com/huggingface/tokenizers/blob/main/tokenizers/src/decoders/byte_fallback.rs

## Setup
For setting up the developer environment you will need following installed:
- [C++ Compiler](https://en.cppreference.com/w/cpp/compiler_support): `gcc` or `clang` is preferred for compiling the source code
- [CMake](https://cmake.org/): build system
- [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html): for formatting the C++ code
- [Common Sense](https://en.wikipedia.org/wiki/Common_sense): essential for working with C++

### Directory Structure

├── [android](../android/): related to Android library using C++ native  
├── [CMakeLists.txt](../CMakeLists.txt): build system configuration  
├── [docs](../docs/): markdown files for providing navigation to the visitors  
├── [examples](../examples/): cross-platform and basic demos  
├── [include](../include/): header files  
├── [ios](../ios/): related to iOS library  
├── [scripts](../scripts/): basic scripts for building dependencies or testing  
├── [src](../src/): source code implementation  
├── [tests](../tests/): unit test cases  
└── [third_party](../third_party/): third party dependencies for JSON parsing, unit testing  

### Building
- This project makes use of 2 third party dependencies:
    - [ICU](https://unicode-org.github.io/icu/): for Unicode related support
    - [simdjson](https://github.com/simdjson/simdjson): for parsing JSON content
- Download PyTorch for examples from: https://download.pytorch.org/libtorch/cpu/ as per your Operation System!
    - Note: Prefer the kind given in [GitHub Actions Workflow](../.github/workflows/main.yml)
- Generate tokenizer config for any HuggingFace model:
```bash
cd scripts/
pip3 install -r requirements.txt
python3 generate_tokenizer_files.py bert-base-uncased
```
This will generate a `tokenizer.json` file at path `<repository root>/data/bert-base-uncased` 
- Build the library, unit test cases and examples:
```bash
mkdir build && cd build
cmake .. && make
```
- Run the examples:
```bash
./examples/run/run ../data/bert-base-uncased 'The goal of the life is [MASK]'
```

### Testing
This project makes use of [GoogleTest](https://github.com/google/googletest) for unit testing. 
The test cases are seperated from source code in the [tests/](../tests/) directory.  
- Run unit test cases:
```bash
ctest --output-on-failure
```