#!/bin/bash

set -e

# Configuration variables
ICU_VERSION="$1"
ICU_SOURCE="icu4c-${ICU_VERSION//-/_}-src"
ANDROID_NDK_HOME="${ANDROID_NDK_HOME:-$HOME/Library/Android/sdk/ndk-bundle}"
ANDROID_PLATFORM=24
CURRENT_DIR="$(pwd)"
BUILD_DIR="${CURRENT_DIR}/build"
DOWNLOAD_DIR="${CURRENT_DIR}/download"
OUTPUT_DIR="${CURRENT_DIR}/output"

# Supported architectures
ANDROID_ARCHS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")
IOS_ARCHS=("arm64" "x86_64")

# Print step information
log_info() {
    echo "==> $1"
}

# Error handling
handle_error() {
    echo "Error: $1"
    exit 1
}

# Create necessary directories
setup_directories() {
    mkdir -p "${DOWNLOAD_DIR}" "${BUILD_DIR}" "${OUTPUT_DIR}"
}

# Download and extract ICU source
download_icu() {
    local url="https://github.com/unicode-org/icu/releases/download/release-${ICU_VERSION}/${ICU_SOURCE}.tgz"
    
    if [ ! -f "${DOWNLOAD_DIR}/${ICU_SOURCE}.tgz" ]; then
        log_info "Downloading ICU ${ICU_VERSION}"
        curl -L "${url}" -o "${DOWNLOAD_DIR}/${ICU_SOURCE}.tgz" || handle_error "Failed to download ICU"
    fi
    
    if [ ! -d "${BUILD_DIR}/icu" ]; then
        log_info "Extracting ICU source"
        tar -xzf "${DOWNLOAD_DIR}/${ICU_SOURCE}.tgz" -C "${BUILD_DIR}" || handle_error "Failed to extract ICU"
    fi
}

# Patch ICU source for iOS compatibility
patch_icu_source() {
    log_info "Patching ICU source for iOS compatibility"
    
    # Create a patch file for pkgdata.cpp
    cat > "${BUILD_DIR}/pkgdata.patch" << 'EOF'
--- a/source/tools/pkgdata/pkgdata.cpp
+++ b/source/tools/pkgdata/pkgdata.cpp
@@ -553,6 +553,10 @@
     printf("pkgdata: %s\n", cmd);
+#if TARGET_OS_IPHONE
+    int result = 0; // Skip system() calls on iOS
+#else
     int result = system(cmd);
+#endif
     if (result != 0) {
         fprintf(stderr, "-- return status = %d\n", result);
         result = 1; // system() result code is platform specific.
     }
EOF
    
    # Apply the patch
    patch -p1 -d "${BUILD_DIR}/icu" < "${BUILD_DIR}/pkgdata.patch" || true
}

# Build ICU for Android
build_android() {
    log_info "Building ICU for Android"
    
    for arch in "${ANDROID_ARCHS[@]}"; do
        log_info "Building for Android ${arch}"
        
        local target_host
        local android_arch
        local extra_flags
        
        case ${arch} in
            "arm64-v8a")
                target_host="aarch64-linux-android"
                android_arch="arm64"
                extra_flags="-fPIC"
                ;;
            "armeabi-v7a")
                target_host="armv7a-linux-androideabi"
                android_arch="arm"
                extra_flags="-fPIC -mthumb"
                ;;
            "x86")
                target_host="i686-linux-android"
                android_arch="x86"
                extra_flags="-fPIC"
                ;;
            "x86_64")
                target_host="x86_64-linux-android"
                android_arch="x86_64"
                extra_flags="-fPIC"
                ;;
        esac
        
        local build_dir="${BUILD_DIR}/android_${arch}"
        mkdir -p "${build_dir}"
        
        local toolchain="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/darwin-x86_64"
        local sysroot="${toolchain}/sysroot"

        # Clean previous build
        rm -rf "${build_dir}"/*
        
        (cd "${build_dir}" && \
         CFLAGS="-Os ${extra_flags}" \
         CXXFLAGS="-Os ${extra_flags}" \
         CC="${toolchain}/bin/${target_host}${ANDROID_PLATFORM}-clang" \
         CXX="${toolchain}/bin/${target_host}${ANDROID_PLATFORM}-clang++" \
         "${BUILD_DIR}/icu/source/configure" \
         --prefix="${OUTPUT_DIR}/android/${arch}" \
         --host="${target_host}" \
         --with-cross-build="${BUILD_DIR}/host" \
         --enable-static \
         --disable-shared \
         --with-data-packaging=static \
         --disable-tests \
         --disable-samples) || handle_error "Failed to configure ICU for Android ${arch}"
        
        make -C "${build_dir}" -j$(sysctl -n hw.ncpu) || handle_error "Failed to build ICU for Android ${arch}"
        make -C "${build_dir}" install || handle_error "Failed to install ICU for Android ${arch}"
    done
}

# Build ICU for iOS
build_ios() {
    log_info "Building ICU for iOS"
    
    local developer=$(xcode-select -print-path)
    local sdk_version=$(xcrun -sdk iphoneos --show-sdk-version)
    
    for arch in "${IOS_ARCHS[@]}"; do
        log_info "Building for iOS ${arch}"
        
        local platform
        local sdk
        
        if [ "${arch}" == "x86_64" ]; then
            platform="iPhoneSimulator"
            sdk="iphonesimulator"
        else
            platform="iPhoneOS"
            sdk="iphoneos"
        fi
        
        local build_dir="${BUILD_DIR}/ios_${arch}"
        mkdir -p "${build_dir}"
        
        local sysroot=$(xcrun --sdk ${sdk} --show-sdk-path)
        
        # Additional flags for iOS build
        local extra_flags="-DUCONFIG_NO_FILE_IO=1 -DU_DISABLE_RENAMING=1 -DUCONFIG_NO_LEGACY_CONVERSION=1"
        
        # Clean previous build
        rm -rf "${build_dir}"/*
        
        (cd "${build_dir}" && \
         CFLAGS="-arch ${arch} -isysroot ${sysroot} -mios-version-min=11.0 -Os ${extra_flags} -DTARGET_OS_IPHONE=1" \
         CXXFLAGS="-arch ${arch} -isysroot ${sysroot} -mios-version-min=11.0 -Os ${extra_flags} -DTARGET_OS_IPHONE=1" \
         LDFLAGS="-arch ${arch} -isysroot ${sysroot}" \
         "${BUILD_DIR}/icu/source/configure" \
         --prefix="${OUTPUT_DIR}/ios/${arch}" \
         --host="${arch}-apple-darwin" \
         --with-cross-build="${BUILD_DIR}/host" \
         --enable-static \
         --disable-shared \
         --with-data-packaging=static \
         --disable-tests \
         --disable-samples \
         --disable-dyload \
         --disable-extras \
         --disable-tools \
         --with-data-packaging=archive) || handle_error "Failed to configure ICU for iOS ${arch}"
        
        make -C "${build_dir}" -j$(sysctl -n hw.ncpu) || handle_error "Failed to build ICU for iOS ${arch}"
        make -C "${build_dir}" install || handle_error "Failed to install ICU for iOS ${arch}"
    done
}

# Build host ICU (required for cross-compilation)
build_host_icu() {
    log_info "Building host ICU"
    
    local host_build="${BUILD_DIR}/host"
    mkdir -p "${host_build}"

    # Clean previous build
    rm -rf "${host_build}"/*
    
    (cd "${host_build}" && \
     "${BUILD_DIR}/icu/source/configure" \
     --disable-tests \
     --disable-samples) || handle_error "Failed to configure host ICU"
    
    make -C "${host_build}" -j$(sysctl -n hw.ncpu) || handle_error "Failed to build host ICU"
}

# Main build process
main() {
    setup_directories
    download_icu
    patch_icu_source
    build_host_icu
    build_android
    build_ios
    
    log_info "Build completed successfully!"
    log_info "Output libraries can be found in: ${OUTPUT_DIR}"
}

# Execute main function
main