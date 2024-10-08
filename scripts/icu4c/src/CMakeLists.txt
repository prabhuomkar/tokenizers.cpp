cmake_minimum_required(VERSION 3.22)

project(ICU4C_SOURCE_BUILD)

include(EnforceSystemEnv)
include(FindGnuMake)
include(ExternalProject)
include(ProcessorCount)

set(ICU_SOURCE_REPO_DIR "${CMAKE_CURRENT_BINARY_DIR}/icu_source_repo")
ExternalProject_Add(
    icu_source_repo
    GIT_REPOSITORY https://github.com/unicode-org/icu
    GIT_TAG release-${ICU_VERSION_MAJOR}-${ICU_VERSION_MINOR}
    GIT_SHALLOW TRUE
    SOURCE_DIR "${ICU_SOURCE_REPO_DIR}"
    UPDATE_DISCONNECTED TRUE
    PATCH_COMMAND
        patch "${ICU_SOURCE_REPO_DIR}/icu4c/source/common/unicode/uconfig.h"
            < "${CMAKE_CURRENT_SOURCE_DIR}/patches/uconfig_h.patch"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

add_library(icu4c INTERFACE)

foreach(lib ${ICU_LIBS})
    add_library(icu${lib} ${ICU_LIBRARY_TYPE} IMPORTED GLOBAL)
    add_library(ICU::${lib} ALIAS icu${lib})
    add_dependencies(icu${lib} icu4c)
endforeach()

if(BUILD_SHARED_LIBS)
    set(ICU_LIB_PREFIX ${ICU_LIBRARY_PREFIX})
    set(ICU_LIB_SUFFIX .${ICU_VERSION_MAJOR})
    set(configure_args_prefix --enable-shared --disable-static)
    set(cpp_flags_prefix)
else()
    set(ICU_LIB_PREFIX ${ICU_LIBRARY_PREFIX})
    set(configure_args_prefix --disable-shared --enable-static)
    set(cpp_flags_prefix -DU_STATIC_IMPLEMENTATION)
endif()
set(ICU_CONFIGURE_ARGS
    ${configure_args_prefix} --disable-debug --enable-release --enable-strict --disable-auto-cleanup
    --disable-draft --disable-renaming --disable-tracing --disable-plugins --disable-extras --enable-icuio
    --disable-layoutex --enable-tools --disable-fuzzer --disable-tests --disable-samples
    --with-data-packaging=archive --with-library-bits=nochange)

set(ICU_C_FLAGS -std=c17 -O3 -fPIC ${ICU_COMPILE_OPTIONS})
set(ICU_CXX_FLAGS -std=c++20 -O3 -fPIC ${ICU_COMPILE_OPTIONS})
set(ICU_CPP_FLAGS ${cpp_flags_prefix})
set(ICU_LDFLAGS ${ICU_LINK_OPTIONS} -pthread)

set(ICU_DATA_FILTER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/data-feature-filter.json)
set(ICU_SRC_DIR "${ICU_SOURCE_REPO_DIR}/icu4c")
if(NOT ICU_BUILD_DIR)
    set(ICU_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()
ProcessorCount(NUM_JOBS)

if(NOT LLVM_TOOLCHAIN)
    get_filename_component(LLVM_TOOLCHAIN ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(LLVM_TOOLCHAIN ${LLVM_TOOLCHAIN} DIRECTORY)
endif()

set(ICU_HOST_BUILD_DIR ${ICU_BUILD_DIR}/.intermediate/host)
set(ICU_HOST_INSTALL_DIR ${ICU_BUILD_DIR}/host)
file(MAKE_DIRECTORY ${ICU_HOST_BUILD_DIR}) # Need to make build directory as ICU script requires this
set(ICU_HOST_ENV_CMAKE
    ${CMAKE_COMMAND} -E env ICU_SOURCES="${ICU_SRC_DIR}" ICU_DATA_FILTER_FILE="${ICU_DATA_FILTER_FILE}"
        CC="${LLVM_TOOLCHAIN}/bin/clang" CXX="${LLVM_TOOLCHAIN}/bin/clang++" CFLAGS="${ICU_C_FLAGS}" CXXFLAGS="${ICU_CXX_FLAGS}"
        CPPFLAGS="${ICU_CPP_FLAGS}" LDFLAGS="${ICU_LDFLAGS}")
set(ICU_HOST_INSTALL_BYPRODUCTS)
foreach(lib ${ICU_LIBS})
    list(APPEND ICU_HOST_INSTALL_BYPRODUCTS
        ${ICU_HOST_INSTALL_DIR}/lib/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
    if(ICU_LIBRARY_OUTPUT_DIRECTORY)
        list(APPEND ICU_HOST_INSTALL_BYPRODUCTS
            ${ICU_LIBRARY_OUTPUT_DIRECTORY}/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
    endif()
endforeach()

add_custom_target(
    icu4c_host_configure
    COMMAND ${ICU_HOST_ENV_CMAKE} ${ICU_SRC_DIR}/source/configure
      -q --prefix=${ICU_HOST_INSTALL_DIR} ${ICU_CONFIGURE_ARGS}
    DEPENDS icu_source_repo
    WORKING_DIRECTORY ${ICU_HOST_BUILD_DIR})
add_custom_target(
    icu4c_host_build
    COMMAND ${ICU_HOST_ENV_CMAKE} ${GNU_MAKE_PROGRAM} -j ${NUM_JOBS} >/dev/null
    DEPENDS icu4c_host_configure
    WORKING_DIRECTORY ${ICU_HOST_BUILD_DIR})
add_custom_target(
    icu4c_host
    COMMAND ${ICU_HOST_ENV_CMAKE} ${GNU_MAKE_PROGRAM} install >/dev/null
    DEPENDS icu4c_host_build
    BYPRODUCTS ${ICU_HOST_INSTALL_BYPRODUCTS}
    WORKING_DIRECTORY ${ICU_HOST_BUILD_DIR})
add_dependencies(icu4c icu4c_host)

set(ICU_LIB_DIR ${ICU_HOST_INSTALL_DIR}/lib)
set(ICU_INCLUDE_DIR ${ICU_HOST_INSTALL_DIR}/include)
set(ICU_TARGET icu4c_host)

if(ANDROID)
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
        set(ICU_ANDROID_TARGET "armv7a-linux-androideabi")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        set(ICU_ANDROID_TARGET "aarch64-linux-android")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
        set(ICU_ANDROID_TARGET "i686-linux-android")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        set(ICU_ANDROID_TARGET "x86_64-linux-android")
    else()
        message(FATAL_ERROR "Unsupported ABI: ${CMAKE_ANDROID_ARCH_ABI}")
    endif()

    set(ICU_ANDROID_BUILD_DIR ${ICU_BUILD_DIR}/.intermediate/android/${CMAKE_ANDROID_ARCH_ABI})
    set(ICU_ANDROID_INSTALL_DIR ${ICU_BUILD_DIR}/android/${CMAKE_ANDROID_ARCH_ABI})
    file(MAKE_DIRECTORY ${ICU_ANDROID_BUILD_DIR}) # Need to make build directory as ICU script requires this
    set(ICU_ANDROID_CC ${LLVM_TOOLCHAIN}/bin/${ICU_ANDROID_TARGET}${CMAKE_ANDROID_API}-clang)
    set(ICU_ANDROID_CXX ${LLVM_TOOLCHAIN}/bin/${ICU_ANDROID_TARGET}${CMAKE_ANDROID_API}-clang++)
    set(ICU_ANDROID_ENV_CMAKE
        ${CMAKE_COMMAND} -E env
        TOOLCHAIN=${LLVM_TOOLCHAIN}
        TARGET=${ICU_ANDROID_TARGET}
        NDK_STANDARD_ROOT=${LLVM_TOOLCHAIN}
        API=${CMAKE_ANDROID_API}
        ANDROIDVER=${CMAKE_ANDROID_API}
        AR=${LLVM_TOOLCHAIN}/bin/llvm-ar
        AS=${ICU_ANDROID_CC}
        CC=${ICU_ANDROID_CC}
        CXX=${ICU_ANDROID_CXX}
        LD=${LLVM_TOOLCHAIN}/bin/ld
        RANLIB=${LLVM_TOOLCHAIN}/bin/llvm-ranlib
        STRIP=${LLVM_TOOLCHAIN}/bin/llvm-strip
        CFLAGS="${ICU_C_FLAGS}"
        CXXFLAGS="${ICU_CXX_FLAGS}"
        CPPFLAGS="${ICU_CPP_FLAGS}"
        LDFLAGS="${ICU_LDFLAGS}"
        ICU_SOURCES=${ICU_SRC_DIR}
        ICU_DATA_FILTER_FILE=${ICU_DATA_FILTER_FILE}
        ICU_CROSS_BUILD=${ICU_HOST_BUILD_DIR}
    )
    set(ICU_ANDROID_INSTALL_BYPRODUCTS)
    foreach(lib ${ICU_LIBS})
        list(APPEND ICU_ANDROID_INSTALL_BYPRODUCTS
            ${ICU_ANDROID_INSTALL_DIR}/lib/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        if(ICU_LIBRARY_OUTPUT_DIRECTORY)
            list(APPEND ICU_ANDROID_INSTALL_BYPRODUCTS
                ${ICU_LIBRARY_OUTPUT_DIRECTORY}/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        endif()
    endforeach()

    add_custom_target(
        icu4c_android_configure
        COMMAND ${ICU_ANDROID_ENV_CMAKE} ${ICU_SRC_DIR}/source/configure --with-cross-build=${ICU_HOST_BUILD_DIR}
            --host=${ICU_ANDROID_TARGET} --prefix=${ICU_ANDROID_INSTALL_DIR} ${ICU_CONFIGURE_ARGS}
        DEPENDS icu4c_host
        WORKING_DIRECTORY ${ICU_ANDROID_BUILD_DIR})
    add_custom_target(
        icu4c_android_build
        COMMAND ${ICU_ANDROID_ENV_CMAKE} ${GNU_MAKE_PROGRAM} -j ${NUM_JOBS} >/dev/null
        DEPENDS icu4c_android_configure
        WORKING_DIRECTORY ${ICU_ANDROID_BUILD_DIR})
    add_custom_target(
        icu4c_android
        COMMAND ${ICU_ANDROID_ENV_CMAKE} ${GNU_MAKE_PROGRAM} install >/dev/null
        DEPENDS icu4c_android_build
        BYPRODUCTS ${ICU_ANDROID_INSTALL_BYPRODUCTS}
        WORKING_DIRECTORY ${ICU_ANDROID_BUILD_DIR})
    add_dependencies(icu4c icu4c_android)

    set(ICU_LIB_DIR ${ICU_ANDROID_INSTALL_DIR}/lib)
    set(ICU_INCLUDE_DIR ${ICU_ANDROID_INSTALL_DIR}/include)
    set(ICU_TARGET icu4c_android)
endif()

if(IOS)
    set(ICU_IOS_BUILD_DIR ${ICU_BUILD_DIR}/.intermediate/ios/${ICU_IOS_ARCH})
    set(ICU_IOS_INSTALL_DIR ${ICU_BUILD_DIR}/ios/${ICU_IOS_ARCH})
    file(MAKE_DIRECTORY ${ICU_IOS_BUILD_DIR}) # Need to make build directory as ICU script requires this
    set(ICU_IOS_CC ${LLVM_TOOLCHAIN}/bin/clang)
    set(ICU_IOS_CXX ${LLVM_TOOLCHAIN}/bin/clang++)
    set(ICU_IOS_ENV_CMAKE
        ${CMAKE_COMMAND} -E env
        TOOLCHAIN=${LLVM_TOOLCHAIN}
        TARGET=${ICU_IOS_ARCH}
        AR=${LLVM_TOOLCHAIN}/bin/llvm-ar
        AS=${ICU_IOS_CC}
        CC=${ICU_IOS_CC}
        CXX=${ICU_IOS_CXX}
        LD=${LLVM_TOOLCHAIN}/bin/ld
        RANLIB=${LLVM_TOOLCHAIN}/bin/llvm-ranlib
        STRIP=${LLVM_TOOLCHAIN}/bin/llvm-strip
        CFLAGS="${ICU_C_FLAGS}"
        CXXFLAGS="${ICU_CXX_FLAGS}"
        CPPFLAGS="${ICU_CPP_FLAGS}"
        LDFLAGS="${ICU_LDFLAGS}"
        ICU_SOURCES=${ICU_SRC_DIR}
        ICU_DATA_FILTER_FILE=${ICU_DATA_FILTER_FILE}
        ICU_CROSS_BUILD=${ICU_HOST_BUILD_DIR}
    )
    set(ICU_IOS_INSTALL_BYPRODUCTS)
    foreach(lib ${ICU_LIBS})
        list(APPEND ICU_IOS_INSTALL_BYPRODUCTS
            ${ICU_IOS_INSTALL_DIR}/lib/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        if(ICU_LIBRARY_OUTPUT_DIRECTORY)
            list(APPEND ICU_IOS_INSTALL_BYPRODUCTS
                ${ICU_LIBRARY_OUTPUT_DIRECTORY}/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        endif()
    endforeach()

    add_custom_target(
        icu4c_IOS_configure
        COMMAND ${ICU_IOS_ENV_CMAKE} ${ICU_SRC_DIR}/source/configure --with-cross-build=${ICU_HOST_BUILD_DIR}
            --host=${ICU_IOS_ARCH} --prefix=${ICU_IOS_INSTALL_DIR} ${ICU_CONFIGURE_ARGS}
        DEPENDS icu4c_host
        WORKING_DIRECTORY ${ICU_IOS_BUILD_DIR})
    add_custom_target(
        icu4c_IOS_build
        COMMAND ${ICU_IOS_ENV_CMAKE} ${GNU_MAKE_PROGRAM} -j ${NUM_JOBS} >/dev/null
        DEPENDS icu4c_IOS_configure
        WORKING_DIRECTORY ${ICU_IOS_BUILD_DIR})
    add_custom_target(
        icu4c_ios
        COMMAND ${ICU_IOS_ENV_CMAKE} ${GNU_MAKE_PROGRAM} install >/dev/null
        DEPENDS icu4c_IOS_build
        BYPRODUCTS ${ICU_IOS_INSTALL_BYPRODUCTS}
        WORKING_DIRECTORY ${ICU_IOS_BUILD_DIR})
    add_dependencies(icu4c icu4c_ios)

    set(ICU_LIB_DIR ${ICU_IOS_INSTALL_DIR}/lib)
    set(ICU_INCLUDE_DIR ${ICU_IOS_INSTALL_DIR}/include)
    set(ICU_TARGET icu4c_ios)
endif()

if(ICU_DESKTOP_TARGET)
    set(ICU_DESKTOP_BUILD_DIR ${ICU_BUILD_DIR}/.intermediate/desktop/${ICU_DESKTOP_TARGET})
    set(ICU_DESKTOP_INSTALL_DIR ${ICU_BUILD_DIR}/desktop/${ICU_DESKTOP_TARGET})
    file(MAKE_DIRECTORY ${ICU_DESKTOP_BUILD_DIR}) # Need to make build directory as ICU script requires this
    set(ICU_DESKTOP_CC ${LLVM_TOOLCHAIN}/bin/clang)
    set(ICU_DESKTOP_CXX ${LLVM_TOOLCHAIN}/bin/clang++)
    set(ICU_DESKTOP_ENV_CMAKE
        ${CMAKE_COMMAND} -E env
        TOOLCHAIN=${LLVM_TOOLCHAIN}
        TARGET=${ICU_DESKTOP_TARGET}
        AR=${LLVM_TOOLCHAIN}/bin/llvm-ar
        AS=${ICU_DESKTOP_CC}
        CC=${ICU_DESKTOP_CC}
        CXX=${ICU_DESKTOP_CXX}
        LD=${LLVM_TOOLCHAIN}/bin/ld
        RANLIB=${LLVM_TOOLCHAIN}/bin/llvm-ranlib
        STRIP=${LLVM_TOOLCHAIN}/bin/llvm-strip
        CFLAGS="${ICU_C_FLAGS}"
        CXXFLAGS="${ICU_CXX_FLAGS}"
        CPPFLAGS="${ICU_CPP_FLAGS}"
        LDFLAGS="${ICU_LDFLAGS}"
        ICU_SOURCES=${ICU_SRC_DIR}
        ICU_DATA_FILTER_FILE=${ICU_DATA_FILTER_FILE}
        ICU_CROSS_BUILD=${ICU_HOST_BUILD_DIR}
    )
    set(ICU_DESKTOP_INSTALL_BYPRODUCTS)
    foreach(lib ${ICU_LIBS})
        list(APPEND ICU_DESKTOP_INSTALL_BYPRODUCTS
            ${ICU_DESKTOP_INSTALL_DIR}/lib/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        if(ICU_LIBRARY_OUTPUT_DIRECTORY)
            list(APPEND ICU_DESKTOP_INSTALL_BYPRODUCTS
                ${ICU_LIBRARY_OUTPUT_DIRECTORY}/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        endif()
    endforeach()

    add_custom_target(
        icu4c_desktop_configure
        COMMAND ${ICU_DESKTOP_ENV_CMAKE} ${ICU_SRC_DIR}/source/configure --with-cross-build=${ICU_HOST_BUILD_DIR}
            --host=${ICU_DESKTOP_TARGET} --prefix=${ICU_DESKTOP_INSTALL_DIR} ${ICU_CONFIGURE_ARGS}
        DEPENDS icu4c_host
        WORKING_DIRECTORY ${ICU_DESKTOP_BUILD_DIR})
    add_custom_target(
        icu4c_desktop_build
        COMMAND ${ICU_DESKTOP_ENV_CMAKE} ${GNU_MAKE_PROGRAM} -j ${NUM_JOBS} >/dev/null
        DEPENDS icu4c_desktop_configure
        WORKING_DIRECTORY ${ICU_DESKTOP_BUILD_DIR})
    add_custom_target(
        icu4c_desktop
        COMMAND ${ICU_DESKTOP_ENV_CMAKE} ${GNU_MAKE_PROGRAM} install >/dev/null
        DEPENDS icu4c_desktop_build
        BYPRODUCTS ${ICU_DESKTOP_INSTALL_BYPRODUCTS}
        WORKING_DIRECTORY ${ICU_DESKTOP_BUILD_DIR})
    add_dependencies(icu4c icu4c_desktop)

    set(ICU_LIB_DIR ${ICU_DESKTOP_INSTALL_DIR}/lib)
    set(ICU_INCLUDE_DIR ${ICU_DESKTOP_INSTALL_DIR}/include)
    set(ICU_TARGET icu4c_desktop)
endif()

file(MAKE_DIRECTORY ${ICU_INCLUDE_DIR})

foreach(lib ${ICU_LIBS})
    set(intermediate_lib_path ${ICU_LIB_DIR}/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
    if(ICU_LIBRARY_OUTPUT_DIRECTORY)
        set(lib_path ${ICU_LIBRARY_OUTPUT_DIRECTORY}/${ICU_LIB_PREFIX}icu${lib}${ICU_LIB_SUFFIX})
        add_custom_command(TARGET ${ICU_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${intermediate_lib_path} ${lib_path})
    endif()
    target_compile_definitions(icu${lib} INTERFACE ${ICU_CPP_FLAGS})
    set_target_properties(icu${lib} PROPERTIES
        IMPORTED_LOCATION ${intermediate_lib_path}
        INTERFACE_INCLUDE_DIRECTORIES ${ICU_INCLUDE_DIR})
endforeach()

if(ICU_ASSET_EXPORT_DIR)
    set(src_asset_dir ${ICU_HOST_INSTALL_DIR}/share/icu/${ICU_VERSION_MAJOR}.${ICU_VERSION_MINOR})
    set(dst_asset_dir ${ICU_ASSET_EXPORT_DIR})
    add_custom_command(TARGET ${ICU_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${src_asset_dir}/icudt${ICU_VERSION_MAJOR}l.dat
            ${dst_asset_dir}/icudt${ICU_VERSION_MAJOR}l.dat)
    add_custom_command(TARGET ${ICU_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${src_asset_dir}/LICENSE
            ${dst_asset_dir}/LICENSE)
endif()

if(ICU_PREBUILT_TARGET)
    add_custom_target(icu4c_prebuilt_target ALL)
    add_dependencies(icu4c_prebuilt_target icu4c)
endif()
