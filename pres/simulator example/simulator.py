from IPython.core.magic import register_cell_magic
from pygdbmi.gdbcontroller import GdbController
from pprint import pprint

from typing import List, Optional
from argparse import ArgumentParser

class GDBSimulator:
    def __init__(self) -> None:
        self.file = None
        self.gdb = GdbController()

    def load(self, file: str) -> None:
        if file == self.file:
            return
        self.file = file
        self.restart()
        return self.run(f'file {self.file}')

    def restart(self) -> None:
        self.gdb.spawn_new_gdb_subprocess()

    def run(self, cmd: str):
        return self.gdb.write(cmd)


sim = GDBSimulator()


def print_console_payload(response):
    for part in response:
        if part['type'] == 'console':
            print(part['payload'].encode('utf-8').decode('unicode_escape'), end='')


@register_cell_magic('lart.simulator')
def simulator(line, cell):
    args = line.split()
    cmds = cell.splitlines()

    parser = ArgumentParser(description="lart simulator")
    parser.add_argument('file', metavar='path', type=str, help='file to simulate')
    parsed = parser.parse_args(args)

    response = sim.load(parsed.file)
    if response:
        print_console_payload(response)

    for cmd in cmds:
        print_console_payload(sim.run(cmd))
