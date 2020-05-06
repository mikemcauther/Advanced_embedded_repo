#!/bin/bash

PROJ_REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

EI_ROOT_PATH=${PROJ_REPO_ROOT}/ei-freertos
EI_CHANGE_PATH=${PROJ_REPO_ROOT}/ei-changes

FILES=$(cd "${EI_ROOT_PATH}" && git status)

OLD_IFS=${IFS}
IFS=$'\n'
for PER_FILE in ${FILES}
do
    if [ ${PER_FILE} != "${PER_FILE/modified\:/}" ]; then
        FILE_PATH="$(echo "${PER_FILE}" | sed  's/^.*modified:[[:blank:]]*\([^\s]\)/\1/' )"
        if [ -f "${EI_ROOT_PATH}/${FILE_PATH}" ]; then
            if [ -f "${EI_CHANGE_PATH}/${FILE_PATH}" ]; then
                echo ""
                echo "Overrite FILE: ${EI_CHANGE_PATH}/${FILE_PATH}"
                diff "${EI_ROOT_PATH}/${FILE_PATH}" "${EI_CHANGE_PATH}/${FILE_PATH}"
                cp -f "${EI_ROOT_PATH}/${FILE_PATH}" "${EI_CHANGE_PATH}/${FILE_PATH}"
            else
                FILE_EI_CHANGE_DIR_PATH=$(dirname "${EI_CHANGE_PATH}/${FILE_PATH}")
                # Check Path exist
                if [ ! -d "${FILE_EI_CHANGE_DIR_PATH}" ]; then
                    echo "Add new folder: ${FILE_EI_CHANGE_DIR_PATH}"
                    mkdir -p ${FILE_EI_CHANGE_DIR_PATH}
                fi
                echo "Add new file: ${EI_CHANGE_PATH}/${FILE_PATH}"
                cp "${EI_ROOT_PATH}/${FILE_PATH}" "${FILE_EI_CHANGE_DIR_PATH}/"
            fi
        fi
    fi
done

IFS=${OLD_IFS}
