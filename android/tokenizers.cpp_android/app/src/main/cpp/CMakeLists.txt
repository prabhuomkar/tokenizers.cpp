cmake_minimum_required(VERSION 3.22.1)

project("tokenizers.cpp_android")

set(TOKENIZERS.CPP_DIR ${CMAKE_SOURCE_DIR}/../../../../../../)

include_directories(${TOKENIZERS.CPP_DIR}/include)
include_directories(${TOKENIZERS.CPP_DIR}/third_party/simdjson/include)
include_directories(${TOKENIZERS.CPP_DIR}/third_party/simdjson/src)

set(TOKENIZERS.CPP_SOURCES
        ${TOKENIZERS.CPP_DIR}/src/added_vocabulary.cpp
        ${TOKENIZERS.CPP_DIR}/src/tokenizer.cpp
        ${TOKENIZERS.CPP_DIR}/src/utils.cpp
        ${TOKENIZERS.CPP_DIR}/src/common.cpp
        ${TOKENIZERS.CPP_DIR}/src/normalizer.cpp
        ${TOKENIZERS.CPP_DIR}/src/pre_tokenizer.cpp
        ${TOKENIZERS.CPP_DIR}/src/model.cpp
        ${TOKENIZERS.CPP_DIR}/src/decoder.cpp
        ${TOKENIZERS.CPP_DIR}/src/post_processor.cpp
        ${TOKENIZERS.CPP_DIR}/third_party/simdjson/src/simdjson.cpp
)

set(ICU_ROOT_PATH ${CMAKE_SOURCE_DIR}/../../../../../../../icu4c/prebuilt)
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
else()
    message(FATAL_ERROR "Unsupported Android ABI: ${ANDROID_ABI}")
endif()


add_library(tokenizers.cpp SHARED ${TOKENIZERS.CPP_SOURCES})
target_link_libraries(tokenizers.cpp ${ICU_LIB_PATH}/libicuuc_floris.a
        ${ICU_LIB_PATH}/libicui18n_floris.a
        ${ICU_LIB_PATH}/libicudata_floris.a)

add_library(${CMAKE_PROJECT_NAME} SHARED tokenizers.cpp_jni.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} tokenizers.cpp android log ${ICU_LIB_PATH}/libicuuc_floris.a
        ${ICU_LIB_PATH}/libicui18n_floris.a
        ${ICU_LIB_PATH}/libicudata_floris.a)
