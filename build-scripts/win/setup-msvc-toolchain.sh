#!/bin/bash

ARCH=${1:?"Missing ARCH argument. ARCH=ARM64 or ARCH=AMD64"}

echo "[INFO] setup-msvc-toolchain.sh: ARCH=$ARCH"

if [[ -z "$CUDA_PATH" ]]; then
    echo "CUDA_PATH is not set. nvenc will not be available."
else
    CUDA_PATH_UNIX=$(cygpath -u "$CUDA_PATH")
    export PATH="${CUDA_PATH_UNIX}/bin/":$PATH
fi

if [[ -z "$VCINSTALLDIR" ]]; then
    echo "Couldn't find Visual Studio install location. Aborting."
    return 1
elif [[ "${ARCH}" == "AMD64" ]]; then
    VCINSTALLDIR_UNIX=$(cygpath -u "$VCINSTALLDIR")
    export PATH="${VCINSTALLDIR_UNIX}/Tools/MSVC/${VCToolsVersion}/bin/Hostx64/x64/":$PATH
    export PATH="${VCINSTALLDIR_UNIX}/../MSBuild/Current/Bin":$PATH
else
    VCINSTALLDIR_UNIX=$(cygpath -u "$VCINSTALLDIR")
    # use this when doing native build
    # export PATH="${VCINSTALLDIR_UNIX}/Tools/MSVC/${VCToolsVersion}/bin/Hostarm64/arm64/":$PATH
    export PATH="${VCINSTALLDIR_UNIX}/Tools/MSVC/${VCToolsVersion}/bin/Hostx64/arm64/":$PATH
    export PATH="${VCINSTALLDIR_UNIX}/../MSBuild/Current/Bin":$PATH
fi

if [[ -z "$WindowsSdkVerBinPath" ]]; then
    echo "WindowsSdkVerBinPath is not set. Aborting."
    return 1
else
    if [[ "${ARCH}" == "AMD64" ]]; then
        WindowsSdkVerBinPath_UNIX=$(cygpath -u "$WindowsSdkVerBinPath")
        export PATH="${WindowsSdkVerBinPath_UNIX}/x64/":$PATH
    elif [[ "${ARCH}" == "ARM64" ]]; then
        WindowsSdkVerBinPath_UNIX=$(cygpath -u "$WindowsSdkVerBinPath")
        # use this when doing native build
        # export PATH="${WindowsSdkVerBinPath_UNIX}/arm64/":$PATH
        export PATH="${WindowsSdkVerBinPath_UNIX}/x64/":$PATH
    fi
fi

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig":$PKG_CONFIG_PATH
echo "[INFO] PKG_CONFIG_PATH set to $PKG_CONFIG_PATH"
echo "[INFO] PATH set to $PATH"

# Add yasm to PATH
if [[ "${ARCH}" == "AMD64" ]]; then
    SCRIPT_DIR_UNIX=$(cygpath -u "$(dirname "$0")")
    export PATH="${SCRIPT_DIR_UNIX}/../../conan/lib3rdparty/nasm/bin":$PATH
fi
