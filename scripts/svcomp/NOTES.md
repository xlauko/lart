## Test Benchexec

`python3 -m benchexec.test_tool_info lart --no-container`

## Run Tool from Submission File

To create submission file execute `setup-submission.sh`.

`./lart --property=<pathto>/sv-benchmarks/c/properties/unreach-call.prp <benchmark>`


## Run Benchexec

from submission folder:

`benchexec ../scripts/svcomp/benchexec.xml --no-container --tool-directory=./ -T 60 -t ReachSafety-BitVectors -N 20`

## Docker run:

`docker run -v /sys/fs/cgroup:/sys/fs/cgroup:rw -v /Users/Casual/src/lart:/usr/src/lart:rw --privileged --cap-drop=all -it lart`
`docker run -v <path-to>/sv-benchmarks/:/usr/set/sv-benchmarks:rw -v /sys/fs/cgroup:/sys/fs/cgroup:rw -v <path-to>/lart:/usr/src/lart:rw --privileged --cap-drop=all -it lart`