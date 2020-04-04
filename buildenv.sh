#!/bin/bash

set +x

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

TOOLCHAIN_PATH=/home/csse3010/projects/csse4011/tools
TOOLCHAIN_DOWNLOAD_URL="https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads"

EI_GIT_ADDRESS="https://github.com/uqembeddedsys/ei-freertos.git"

export EI_ROOT_PATH=${REPO_ROOT}/ei-freertos


# Arm eabi check
if [ -d ${TOOLCHAIN_PATH} ]; then
    echo ${PATH} | grep "${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin" >/dev/null
    if [ $? -eq 1 ]; then
        echo ""
        echo "Try export PATH: ${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin"
        export PATH=${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin:${PATH}
    fi
else
    echo ""
    echo "WARNING: We expect toolchain location in ${TOOLCHAIN_PATH}"
    echo "You can try to download from ${TOOLCHAIN_DOWNLOAD_URL}"
    echo "and install in ${TOOLCHAIN_PATH}"
fi

# Download EI-FREERTOS
if ! [ -d ${EI_ROOT_PATH} ]; then
    echo ""
    echo "Try download ei-freertos"
    cd ${REPO_ROOT}
    git clone ${EI_GIT_ADDRESS}
fi

# Export EI related path
echo ${PYTHONPATH} | grep "${EI_ROOT_PATH}/pyclasses" >/dev/null
if [ $? -eq 1 ]; then
    echo ""
    echo "EXPORT: ${EI_ROOT_PATH}/pyclasses"
    export PYTHONPATH=${EI_ROOT_PATH}/pyclasses:$PYTHONPATH
fi

echo ${PATH} | grep "${EI_ROOT_PATH}/tools" >/dev/null
if [ $? -eq 1 ]; then
    echo ""
    echo "EXPORT: ${EI_ROOT_PATH}/tools"
    export PATH=${EI_ROOT_PATH}/tools:$PATH
fi
