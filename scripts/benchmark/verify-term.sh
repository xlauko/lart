#!/bin/bash

set -e

file=$1

trace_file="trace.out"

export LART_NO_FAIL_MODE=ON
export LART_TRACE_FILE=$trace_file

lartcc term $file -lz3 -o abstracted

./abstracted 2>&1

if [ -s $trace_file ]; then
	echo  {\"result\": \"error\", \"error_description\": \"`cat $trace_file`\"} | tee /artefact/results.json
else
	echo  {\"result\": \"no-error\"} | tee /artefact/results.json
fi

