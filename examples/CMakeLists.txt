cmake_minimum_required(VERSION 3.27)

project(bert-base-uncased VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

find_library(TOKENIZERS_CPP_LIB
  NAMES tokenizers.cpp
  HINTS ../build
)

if(NOT TOKENIZERS_CPP_LIB)
  message(FATAL_ERROR "tokenizers.cpp library not found")
else()
  message(STATUS "Found the library tokenizers.cpp at ${TOKENIZERS_CPP_LIB}")
endif()

include_directories(../include)
include_directories(../third_party/simdjson/include)
find_package(ICU REQUIRED COMPONENTS uc i18n)

add_executable(bert-base-uncased bert-base-uncased.cpp)

target_link_libraries(bert-base-uncased ${TOKENIZERS_CPP_LIB} ICU::uc ICU::i18n)
