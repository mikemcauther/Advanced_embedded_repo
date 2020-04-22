#!/bin/bash

PY_SCRIPT_PATH="$(pwd)/myoslib/scu"
. ./env_set.sh
(pyboard  --device /dev/ttyACM0 -f ls && pyboard --device /dev/ttyACM0 -f cp ${PY_SCRIPT_PATH}/hci.py : )
(pyboard  --device /dev/ttyACM0 -f ls && pyboard --device /dev/ttyACM0 -f cp ${PY_SCRIPT_PATH}/led.py : )
(pyboard  --device /dev/ttyACM0 -f ls && pyboard --device /dev/ttyACM0 -f cp ${PY_SCRIPT_PATH}/main.py : )

(pyboard  --device /dev/ttyACM1 -f ls && pyboard --device /dev/ttyACM1 -f cp ${PY_SCRIPT_PATH}/hci.py : )
(pyboard  --device /dev/ttyACM1 -f ls && pyboard --device /dev/ttyACM1 -f cp ${PY_SCRIPT_PATH}/led.py : )
(pyboard  --device /dev/ttyACM1 -f ls && pyboard --device /dev/ttyACM1 -f cp ${PY_SCRIPT_PATH}/main.py : )
