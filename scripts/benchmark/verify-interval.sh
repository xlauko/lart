#!/bin/bash

set -e

file=$1

trace_file="trace.out"

export LART_NO_FAIL_MODE=ON
export LART_TRACE_FILE=$trace_file

lartcc interval file -o abstracted

./abstracted

if [ -s $trace_file ]; then
	echo  {\"result\": \"error\", \"error_description\": \"`cat $trace_file`\"} > /artefact/results.json
else
	echo  {\"result\": \"no-error\"} > /artefact/results.json
fi

