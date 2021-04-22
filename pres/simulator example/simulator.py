from ipywidgets import interact, interactive, fixed, interact_manual
import ipywidgets as widgets

from sys import stdout, stdin
from subprocess import Popen, PIPE, STDOUT
from threading import Thread

from IPython.display import clear_output

import time

verbose_listener = False

class Listener:
    def __init__(self, channel) -> None:
        self.running: bool = False
        self.channel = channel
        self.listener = None
        self.buffer: List[str] = []

    def loop(self) -> None:
        while self.running:
            line = self.channel.readline().decode(stdout.encoding)
            self.buffer.append(line)
    
    def flush(self) -> None:
        global verbose_listener
        if verbose_listener:
            for line in self.buffer:
                print(line, end='')
        self.buffer = []

    def start(self) -> None:
        self.running = True
        self.listener = Thread(target=self.loop)
        self.listener.start()


class Simulator:
    def __init__(self) -> None:
        self.proc = None
        self.pout = None
        self.pin = None

    def setup_process(self) -> None:
        self.proc = Popen('gdb', stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        self.pout = self.proc.stdout
        self.pin = self.proc.stdin
        self.listener = Listener(self.pout)
        self.listener.start()

    def start(self) -> None:
        global verbose_listener

        self.setup_process()
        
        time.sleep(0.5)
        self.listener.flush()
        verbose_listener = True

        while self.proc.poll() is None:
            cmd = input('command: ')
            if cmd == 'mute':
                print('(sim) muted')
                verbose_listener = False
            elif cmd == 'unmute':
                print('(sim) unmuted')
                verbose_listener = True
            elif self.proc.poll() is None:
                self.pin.write(bytearray(cmd + '\n', stdin.encoding))
                self.pin.flush()
            
            clear_output()
            time.sleep(0.3)
            self.listener.flush()

            if 'quit' in cmd:
                break

    def restart(self) -> None:
        pass