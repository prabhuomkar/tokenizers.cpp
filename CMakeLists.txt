cmake_minimum_required(VERSION 3.27)

project(tokenizers.cpp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

find_package(ICU REQUIRED COMPONENTS uc i18n)

set(SOURCES
  src/added_vocabulary.cpp
  src/tokenizer.cpp
  src/utils.cpp
  src/common.cpp
  src/normalizer.cpp
  src/pre_tokenizer.cpp
  src/model.cpp
  src/decoder.cpp
  src/post_processor.cpp
  third_party/simdjson/src/simdjson.cpp
)

add_library(tokenizers.cpp SHARED ${SOURCES})

target_include_directories(tokenizers.cpp
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdjson/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdjson/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS tokenizers.cpp DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

add_subdirectory(third_party/googletest)

enable_testing()
add_subdirectory(tests)
