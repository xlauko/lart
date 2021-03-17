#!/bin/bash

RUNNER=$1
OPTS="${@:2}"
PROG=${!#}

compile () {
    echo ${PWD}
    ${RUNNER} ${OPTS} -I../../include
}

execute () {
    DFSAN_OPTIONS=warn_unimplemented=0 ${PROG} 2>&1
}

compile; execute
