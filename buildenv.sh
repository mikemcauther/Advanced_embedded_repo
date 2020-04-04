#!/bin/bash

set +x

PROJ_REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

TOOLCHAIN_PATH=/home/csse3010/projects/csse4011/tools
TOOLCHAIN_DOWNLOAD_URL="https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads"

EI_GIT_ADDRESS="https://github.com/uqembeddedsys/ei-freertos.git"

EI_ROOT_PATH=${PROJ_REPO_ROOT}/ei-freertos

# Variable to export in proj_common.mk
#PROJ_PATH =
#PROJ_PYTHONPATH =


# Construct common config 
cd ${PROJ_REPO_ROOT}
rm -f ./common/proj_common.mk
touch ./common/proj_common.mk
echo "EI_ROOT_PATH=${PROJ_REPO_ROOT}/ei-freertos" >> ./common/proj_common.mk

# Arm eabi check
if [ -d ${TOOLCHAIN_PATH} ]; then
    if [ ${PATH} == ${PATH/${TOOLCHAIN_PATH}\/gcc-arm-none-eabi-7-2017-q4-major\/bin/} ]; then
        echo ""
        echo "Add PATH: ${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin"
        PROJ_PATH=${TOOLCHAIN_PATH}/gcc-arm-none-eabi-7-2017-q4-major/bin${PROJ_PATH:+\:${PROJ_PATH}}
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
    cd ${PROJ_REPO_ROOT}
    git clone ${EI_GIT_ADDRESS}
fi

# Export EI related path
if [ ${PYTHONPATH} == ${PYTHONPATH/${EI_ROOT_PATH}\/pyclasses/} ]; then
    echo ""
    echo "Add PYTHONPATH: ${EI_ROOT_PATH}/pyclasses"
    PROJ_PYTHONPATH=${EI_ROOT_PATH}/pyclasses${PROJ_PYTHONPATH:+\:${PROJ_PYTHONPATH}}
fi

if [ ${PATH} == ${PATH/${EI_ROOT_PATH}\/tools/} ]; then
    echo ""
    echo "Add PATH: ${EI_ROOT_PATH}/tools"
    PROJ_PATH=${EI_ROOT_PATH}/tools${PROJ_PATH:+\:${PROJ_PATH}}
fi

# Print command into proj_common.mk
echo "PATH=${PROJ_PATH}:${PATH}" >> ./common/proj_common.mk
echo "PYTHONPATH=${PROJ_PYTHONPATH}${PYTHONPATH:+\:${PYTHONPATH}}" >> ./common/proj_common.mk
echo "REPO_PATH=${EI_ROOT_PATH}" >> ./common/proj_common.mk
