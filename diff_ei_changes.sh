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
        #echo $FILE_PATH
        if [ -f "${EI_ROOT_PATH}/${FILE_PATH}" ]; then
            if [ -f "${EI_CHANGE_PATH}/${FILE_PATH}" ]; then
                TEMP_DIFF=$(diff ${EI_ROOT_PATH}/${FILE_PATH} ${EI_CHANGE_PATH}/${FILE_PATH})
                if [ ${#TEMP_DIFF} -gt 0 ]; then
                    echo ""
                    echo "diff FILE: ${EI_ROOT_PATH}/${FILE_PATH} ${EI_CHANGE_PATH}/${FILE_PATH}"
                    echo "${TEMP_DIFF}"
                fi
            else
                echo ""
                echo "New diff FILE: ${EI_ROOT_PATH}/${FILE_PATH} ${EI_CHANGE_PATH}/${FILE_PATH}"
                (cd "${EI_ROOT_PATH}" && git diff ${FILE_PATH})
            fi
        fi
    fi
done

IFS=${OLD_IFS}





FILES=$(cd "${EI_CHANGE_PATH}" && find ./* -type f)

OLD_IFS=${IFS}
IFS=$'\n'
for PER_FILE in ${FILES}
do
    FILE_PATH="$( echo "${PER_FILE}" | sed  's/^[[:blank:]]*\.\/\([^\s]\)/\1/' )"
    FILE_EI_ROOT_DIR_PATH=$(dirname "${EI_ROOT_PATH}/${FILE_PATH}")
    FILE_EI_CHANGE_DIR_PATH=$(dirname "${EI_CHANGE_PATH}/${FILE_PATH}")
    #echo "${FILE_PATH}"
    echo ""
    if [ -f "${EI_ROOT_PATH}/${FILE_PATH}" ]; then
        if [ -f "${EI_CHANGE_PATH}/${FILE_PATH}" ]; then
            TEMP_DIFF=$(diff ${EI_ROOT_PATH}/${FILE_PATH} ${EI_CHANGE_PATH}/${FILE_PATH})
            if [ ${#TEMP_DIFF} -gt 0 ]; then
                echo ""
                echo "diff FILE: ${EI_ROOT_PATH}/${FILE_PATH} ${EI_CHANGE_PATH}/${FILE_PATH}"
                echo "${TEMP_DIFF}"
            fi
        else
            echo "Not exist in CHANGE FOLDER: ${EI_CHANGE_PATH}/${FILE_PATH}"
        fi
    else
        echo "Not exist in ROOT FOLDER: ${EI_ROOT_PATH}/${FILE_PATH}"
    fi
done

IFS=${OLD_IFS}
