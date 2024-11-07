#!/usr/bin/env bash
icu_version_major=76
icu_version_minor=1
icu_libs=("uc" "tu" "i18n" "io" "data")
android_abi_list=("armeabi-v7a" "arm64-v8a" "x86" "x86_64")
android_api=24
ios_target_list=("x86_64-apple-darwin" "armv8a-apple-darwin")

cd "$(realpath "$(dirname "$0")")" || exit 1

if [ "$1" = "android" ]  || [ "$1" = "" ]; then
    echo "[Android] Using SDK @ $ANDROID_SDK_ROOT"
    llvm_toolchain="$ANDROID_SDK_ROOT/ndk"
    llvm_toolchain=$(find "$llvm_toolchain" -mindepth 1 -maxdepth 1 -type d | sort -V | tail -n 1)
    echo "[Android] Using NDK @ $llvm_toolchain"
    llvm_toolchain="$llvm_toolchain/toolchains/llvm/prebuilt/linux-$(uname -m)"
    echo "[Android] Using LLVM toolchain @ $llvm_toolchain"
fi
if [ "$1" = "ios" ] || [ "$1" = "" ]; then
    ios_llvm_toolchain="$(realpath $(dirname $(realpath /usr/bin/clang))/..)"
    echo "[iOS] Using LLVM toolchain @ $ios_llvm_toolchain"
fi

echo "Removing $(realpath ./prebuilt)"
rm -rf ./prebuilt 2>/dev/null
echo "Removing $(realpath ./build)"
rm -rf ./build 2>/dev/null

echo
mkdir -p "build"
if [ "$1" = "android" ] || [ "$1" = "" ]; then
    for abi in "${android_abi_list[@]}"; do
        echo "Process target android/$abi"
        echo -n "Configuring..."
        cmake -B=build -S=. \
            -DCMAKE_CXX_FLAGS="-w" -DCMAKE_C_FLAGS="-w" \
            -DANDROID=1 \
            -DIOS=0 \
            -DCMAKE_ANDROID_API=$android_api \
            -DCMAKE_ANDROID_ARCH_ABI=$abi \
            -DICU_DESKTOP_TARGET= \
            -DICU_BUILD_FROM_SOURCE=1 \
            -DICU_PREBUILT_TARGET=1 \
            -DLLVM_TOOLCHAIN="$llvm_toolchain"
        if [ $? -eq 0 ]; then
            echo "Completed"
        else
            echo "Failed ($?)"
            continue
        fi
        echo -n "Building..."
        cmake --build build
        if [ $? -eq 0 ]; then
            echo "Completed"
        else
            echo "Failed ($?)"
        fi
    done
fi
if [ "$1" = "ios" ] || [ "$1" = "" ]; then
    for target in "${ios_target_list[@]}"; do
        echo "Process target ios/$target"
        echo -n "  Configuring..."
        cmake -B=build -S=. \
            -DCMAKE_CXX_FLAGS="-w" -DCMAKE_C_FLAGS="-w" \
            -DANDROID=0 \
            -DIOS=1 \
            -DCMAKE_ANDROID_API= \
            -DCMAKE_ANDROID_ARCH_ABI= \
            -DICU_IOS_ARCH=$target \
            -DICU_DESKTOP_TARGET= \
            -DICU_BUILD_FROM_SOURCE=1 \
            -DICU_PREBUILT_TARGET=1 \
            -DLLVM_TOOLCHAIN="$ios_llvm_toolchain"
        if [ $? -eq 0 ]; then
            echo "Completed"
        else
            echo "Failed ($?)"
            continue
        fi
        echo -n "  Building..."
        cmake --build build
        if [ $? -eq 0 ]; then
            echo "   Completed"
        else
            echo "Failed ($?)"
        fi
    done
fi

echo
if [ "$1" = "android" ] || [ "$1" = "" ]; then
    prebuilt_target_dir="prebuilt/libs/android"
    mkdir -p $prebuilt_target_dir
    echo "Installing Android libs to $(realpath $prebuilt_target_dir)"
    for abi in "${android_abi_list[@]}"; do
        mkdir -p $prebuilt_target_dir/$abi
        for icu_lib in "${icu_libs[@]}"; do
            cp build/src/android/$abi/lib/libicu${icu_lib}.a \
                $prebuilt_target_dir/$abi/libicu${icu_lib}.a
        done
    done
fi
if [ "$1" = "ios" ] || [ "$1" = "" ]; then
    prebuilt_target_dir="prebuilt/libs/ios"
    mkdir -p $prebuilt_target_dir
    echo "Installing iOS libs to $(realpath $prebuilt_target_dir)"
    for target in "${ios_target_list[@]}"; do
        mkdir -p $prebuilt_target_dir/$target
        for icu_lib in "${icu_libs[@]}"; do
            cp build/src/ios/$target/lib/libicu${icu_lib}.a \
                $prebuilt_target_dir/$target/libicu${icu_lib}.a
        done
    done
fi
prebuilt_target_dir="prebuilt/assets/icu"
mkdir -p $prebuilt_target_dir
echo "Installing ICU data file to $(realpath $prebuilt_target_dir)"
mkdir -p $prebuilt_target_dir
cp build/src/host/share/icu/$icu_version_major.$icu_version_minor/icudt${icu_version_major}l.dat \
    $prebuilt_target_dir/icudt${icu_version_major}l.dat
prebuilt_target_dir="prebuilt/include"
mkdir -p $prebuilt_target_dir
echo "Installing ICU header files to $(realpath $prebuilt_target_dir)"
mkdir -p $prebuilt_target_dir
cp -r build/src/host/include/* $prebuilt_target_dir
