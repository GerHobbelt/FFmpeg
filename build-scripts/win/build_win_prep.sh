#!/bin/bash

set -e

ARCH=${1:?"Missing ARCH argument. ARCH=ARM64 or ARCH=AMD64"}

# Ensure all expected system dependencies
pacman -S --noconfirm base-devel pkg-config

# Navigate to sources directory
cd "$(dirname "$0")/../../../"
# Navigate to FFmpeg directory (from nv-codec-headers directory)
cd "./FFmpeg"

echo "Building bin2c.exe for x64 host..."

# Always use AMD64 toolchain for building bin2c.exe (even when target is ARM64)
source ./build-scripts/win/setup-msvc-toolchain.sh AMD64

# Configure minimal build just to get bin2c for x64
./configure --toolchain=msvc --stdc=c17 --arch=x86_64 --disable-all --enable-libzimg --extra-cflags="-I./conan-x64/lib3rdparty/zimg/include" --extra-ldflags="-libpath:./conan-x64/lib3rdparty/zimg/lib"
make ffbuild/bin2c.exe

# Save the x64 bin2c
cp ffbuild/bin2c.exe ffbuild/bin2c_x64.exe
