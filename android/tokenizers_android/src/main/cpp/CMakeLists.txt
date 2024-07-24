cmake_minimum_required(VERSION 3.22.1)

project("tokenizers_android")

set(TOKENIZERS_ROOT_PATH ${CMAKE_SOURCE_DIR}/../../../../../)

include_directories(${TOKENIZERS_ROOT_PATH}/include)
include_directories(${TOKENIZERS_ROOT_PATH}/third_party/simdjson/include)
include_directories(${TOKENIZERS_ROOT_PATH}/third_party/simdjson/src)

set(TOKENIZERS_SOURCES
        ${TOKENIZERS_ROOT_PATH}/src/added_vocabulary.cpp
        ${TOKENIZERS_ROOT_PATH}/src/tokenizer.cpp
        ${TOKENIZERS_ROOT_PATH}/src/utils.cpp
        ${TOKENIZERS_ROOT_PATH}/src/common.cpp
        ${TOKENIZERS_ROOT_PATH}/src/normalizer.cpp
        ${TOKENIZERS_ROOT_PATH}/src/pre_tokenizer.cpp
        ${TOKENIZERS_ROOT_PATH}/src/model.cpp
        ${TOKENIZERS_ROOT_PATH}/src/decoder.cpp
        ${TOKENIZERS_ROOT_PATH}/src/post_processor.cpp
        ${TOKENIZERS_ROOT_PATH}/third_party/simdjson/src/simdjson.cpp
)

set(ICU_ROOT_PATH ${TOKENIZERS_ROOT_PATH}/scripts/icu4c/prebuilt)
set(ICU_INCLUDE_PATH "${ICU_ROOT_PATH}/include")

include_directories(${ICU_INCLUDE_PATH})

set(ICU_LIB_PATH "")
if (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a")
    set(ICU_LIB_PATH "${ICU_ROOT_PATH}/libs/android/armeabi-v7a")
elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
    set(ICU_LIB_PATH "${ICU_ROOT_PATH}/libs/android/arm64-v8a")
elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "x86")
    set(ICU_LIB_PATH "${ICU_ROOT_PATH}/libs/android/x86")
elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "x86_64")
    set(ICU_LIB_PATH "${ICU_ROOT_PATH}/libs/android/x86_64")
endif()

add_library(tokenizers SHARED ${TOKENIZERS_SOURCES})
target_link_libraries(tokenizers ${ICU_LIB_PATH}/libicuuc.a
        ${ICU_LIB_PATH}/libicui18n.a
        ${ICU_LIB_PATH}/libicudata.a)

add_library(${CMAKE_PROJECT_NAME} SHARED tokenizers_android_jni.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} tokenizers android log ${ICU_LIB_PATH}/libicuuc.a
        ${ICU_LIB_PATH}/libicui18n.a
        ${ICU_LIB_PATH}/libicudata.a)
