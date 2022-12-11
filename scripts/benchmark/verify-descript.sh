#!/bin/bash

set -e

domain=$1

RED='\033[0;31m'
NC='\033[0m' # No Color

if [ -z $domain ]; then
     echo -e "${RED}[LART ERROR]${NC} No domain provided. Please specify a domain name."
     exit 1
fi

trace_file="trace.out"

export LART_NO_FAIL_MODE=ON
export LART_TRACE_FILE=$trace_file

cat /usr/src/lart/builds/ninja-surveyor/lartcc/lartcc

echo "COMPILE CODE"
lartcc $domain ${@:2} -o abstracted 2>&1

echo "RUN CODE"
./abstracted 2>&1

if [ -s $trace_file ]; then
    echo  {\"result\": \"`cat $trace_file`\"} > /artefact/results.json
else
    echo  {\"result\": \"correct\"} > /artefact/results.json
fi

echo "TRACE.OUT"
cat $trace_file
echo "RESULTS.JSON"
cat /artefact/results.json
echo ""
if [ -s abstracted ]; then
    echo  "abstracted existuje"
else
    echo  "abstracted neexistuje"
fi
