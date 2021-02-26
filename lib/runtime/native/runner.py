#!/usr/bin/python

import os
import argparse
import pathlib
import tempfile
import shutil

from typing import List

from subprocess import Popen, DEVNULL

class instance(object):
    def __init__(self, program: str, path: str):
        self.program = program
        self.path    = path
        # workdir paths
        self.work_dir     = tempfile.mkdtemp()
        self.stderr : str = self.work_dir + "/exec.err"
        self.stdout : str = self.work_dir + "/exec.out"

    def run(self):
        with open(self.stderr, "w") as err, open(self.stdout, "w") as out:
            self.process = Popen(self.program, stdout=out, stderr=err)

    def choices_taken(self) -> List[int]:
        choices = []
        with open(self.stderr, "r") as err:
            for line in err:
                if line.startswith( "[lart-choice]" ):
                    choices.append( int(line.split(": ")[1]) )
        return choices

    def __del__(self):
        shutil.rmtree(self.work_dir)


def execute_with_path(program, path):
    os.environ["LART_PATH"] = path
    inst = instance(program, path)

    print( f"running: {path}" )
    inst.run()
    inst.process.wait()

    return inst.choices_taken()


def choices_to_path(choices: List[int]) -> List[str]:
    return "-".join( [str(c) for c in choices] )


def execute(program):
    choices = []
    depth = 0

    taken = execute_with_path( program, choices_to_path(choices) )
    while len(taken) > depth:
        taken[depth] += 1
        depth += 1
        taken = execute_with_path( program, choices_to_path(taken) )


def main():
    parser = argparse.ArgumentParser(description='Run abstract execution.')
    parser.add_argument('program', help='path to transformed program', type=str)
    opts = parser.parse_args()
    execute(opts.program)


if __name__ == "__main__":
    main()
