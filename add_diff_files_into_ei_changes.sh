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
        FILE_PATH="$(echo "${PER_FILE}" | sed  's/modified\:[\\s]*\([^\\s]\)/\1/' )"
        echo $FILE_PATH
        if [ -f ${EI_ROOT_PATH}/${FILE_PATH} ]; then
            if [ ! -f ${EI_CHANGE_PATH}/${FILE_PATH} ]; then
                echo ""
                echo "Add FILE: ${EI_CHANGE_PATH}/${FILE_PATH}"
                cp ${EI_ROOT_PATH}/${FILE_PATH} ${EI_CHANGE_PATH}/${FILE_PATH}
            fi
        fi
    fi
done

IFS=${OLD_IFS}
