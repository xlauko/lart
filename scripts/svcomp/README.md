## Test Benchexec

`python3 -m benchexec.test_tool_info scripts.svcomp.tools.lart --no-container`

## Run Tool

`./build/lartcc/lartcc /usr/set/sv-benchmarks/c/properties/unreach-call.prp /usr/set/sv-benchmarks/c/bitvector/byte_add-1.c`

## Run Benchexec

`benchexec ./scripts/svcomp/benchexec.xml --no-container --tool-directory=./scripts/svcomp/`

## Docker run:

`docker run -v /sys/fs/cgroup:/sys/fs/cgroup:rw -v /Users/Casual/src/lart:/usr/src/lart:rw --privileged --cap-drop=all -it lart`