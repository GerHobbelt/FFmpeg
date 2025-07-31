#!/bin/bash

rm -rf ./conan

ARCH=${1:?"Missing ARCH argument. ARCH=ARM64 or ARCH=X86_64"}

if [[ "${ARCH}" == "ARM64" ]]; then
    conan install ./build-scripts/conanfile.py -u -pr:b ./build-scripts/win/profile_win2022 -pr:h ./build-scripts/win/profile_win2022_armv8 -of ./conan
elif [[ "${ARCH}" == "AMD64" ]]; then
    conan install ./build-scripts/conanfile.py -u -pr:b ./build-scripts/win/profile_win2022 -pr:h ./build-scripts/win/profile_win2022 -of ./conan
fi
