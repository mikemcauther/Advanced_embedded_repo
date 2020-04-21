#!/bin/bash

PROJ_REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

EI_ROOT_PATH=${PROJ_REPO_ROOT}/ei-freertos
EI_CHANGE_PATH=${PROJ_REPO_ROOT}/ei-changes

#FILES=$(cd "${EI_ROOT_PATH}" && git status)
FILES=$(cd "${EI_CHANGE_PATH}" && find ./* -type f)

OLD_IFS=${IFS}
IFS=$'\n'
for PER_FILE in ${FILES}
do
    FILE_PATH="$( echo "${PER_FILE}" | sed  's/^[[:blank:]]*\.\/\([^\s]\)/\1/' )"
    echo "${FILE_PATH}"
    if [ -f "${EI_CHANGE_PATH}/${FILE_PATH}" ]; then
        # Check Path exist
        FILE_DIR_PATH=$(dirname "${FILE_PATH}")
        if [ ! -d "${FILE_DIR_PATH}" ]; then
            echo "Add new folder: ${FILE_DIR_PATH}"
            mkdir -p ${FILE_DIR_PATH}
        fi
        if [ ! -f "${FILE_PATH}" ]; then
            echo "Add new file: ${EI_ROOT_PATH}/${FILE_PATH}"
            cp "${EI_CHANGE_PATH}/${FILE_PATH}" "${EI_ROOT_PATH}/${FILE_PATH}"
        else
            echo "Overrite FILE: ${EI_ROOT_PATH}/${FILE_PATH}"
            diff "${EI_ROOT_PATH}/${FILE_PATH}" "${EI_CHANGE_PATH}/${FILE_PATH}"
            cp "${EI_CHANGE_PATH}/${FILE_PATH}" "${EI_ROOT_PATH}/${FILE_PATH}"
        fi
    fi
done

IFS=${OLD_IFS}
