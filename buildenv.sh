#!/bin/bash

set +x

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SOURCELIB_ROOT=$HOME/projects/csse4011/sourcelib
TOOLCHAIN_PATH=/home/csse3010/projects/csse4011/tools
EI_GIT_ADDRESS="https://github.com/uqembeddedsys/ei-freertos.git"
EI_ROOT_PATH=${REPO_ROOT}/ei-freertos

# Arm eabi check
if [ -d ${TOOLCHAIN_PATH} ]; then
    echo ""
    echo "Try export PATH: ${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin"
    export PATH=${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin:${PATH}
fi

# Download EI-FREERTOS
if ! [ -d ${EI_ROOT_PATH} ]; then
    echo ""
    echo "Try download ei-freertos"
    cd ${REPO_ROOT}
    git clone ${EI_GIT_ADDRESS}
fi

# Export EI related path
echo ${PYTHONBPATH} | grep "${EI_ROOT_PATH}/pyclasses" || (export PYTHONPATH=${EI_ROOT_PATH}/pyclasses:$PYTHONPATH)
echo ${PATH} | grep "${EI_ROOT_PATH}/tools" || (export PATH=${EI_ROOT_PATH}/tools:$PATH)
