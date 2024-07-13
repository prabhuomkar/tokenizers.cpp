cmake_minimum_required(VERSION 3.27)

project(bert-base-uncased VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

find_library(TOKENIZERS_CPP_LIB
  NAMES tokenizers.cpp
  HINTS ../build
)

# Verify that the library was found
if(NOT TOKENIZERS_CPP_LIB)
  message(FATAL_ERROR "tokenizers.cpp library not found")
else()
  message(STATUS "tokenizers.cpp library found at ${TOKENIZERS_CPP_LIB}")
endif()

include_directories(../include)
include_directories(../third_party/simdjson/include)

add_executable(bert-base-uncased bert-base-uncased.cpp)

target_link_libraries(bert-base-uncased ${TOKENIZERS_CPP_LIB})
