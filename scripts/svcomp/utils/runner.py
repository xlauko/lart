#!/usr/bin/env python3

import re
import sys
import time
import copy
import os.path

from enum import Enum
from subprocess import Popen, DEVNULL

from typing import List

from . log import logger
from . config import config
from . result import analysis_result, get_result_class, result_class


class approximation(Enum):
    precise = "precise"
    over    = "over"
    under   = "under"


class compilation(object):
    def __init__(self, cfg: config, preprocesed: str):
        self.cfg = cfg
        self.preprocesed : str = preprocesed
        self.err : str = cfg.workdir + "/lartcc.err"
        self.out : str = cfg.workdir + "/lartcc.out"
        self.res : str = cfg.workdir + "/abstracted"

    def get_instance(self):
        cc = os.path.abspath(self.cfg.lartcc)

        cmd = [cc, self.cfg.lamp, self.preprocesed]
        if self.cfg.lamp == "term":
            cmd.append("-lz3")

        # TODO: usupported by dataflow sanitizer
        # if self.cfg.architecture == 32:
        #     cmd.append("-m32")

        # for backtrace
        cmd += ["-rdynamic"]
        # cmd += ["-Os"]
        cmd += ["-o", self.res]

        return cmd

    def run(self):
        cc = self.get_instance()
        
        logger().info("cc: " + " ".join(cc))
        with open(self.out, 'w') as out, open(self.err, 'w') as err:
            proc = Popen(cc, stdout = out, stderr = err)
            status = proc.wait()

        return self.out, self.err, self.res


class verifier(object):
    def __init__(self, cfg: config, abstracted: str):
        self.cfg = cfg
        self.abstracted : str = abstracted
        self.err : str = cfg.workdir + "/verify.err"
        self.out : str = cfg.workdir + "/verify.out"

    def get_instance(self):
        inst = os.path.abspath(self.abstracted)
        return [inst]

    def run(self):
        prog = self.get_instance()
        
        logger().info("verify: " + " ".join(prog))
        with open(self.out, 'w') as out, open(self.err, 'w') as err:
            proc = Popen(prog, stdout = out, stderr = err)
            status = proc.wait()

        return self.out, self.err


class runner(object):
    def __init__(self, cfg: config):
        self.cfg : config = cfg
        self.preprocesed : str = cfg.workdir + "/benchmark.c"

    def preprocess(self):
        with open(self.preprocesed, "w") as out, open(self.cfg.benchmark, "r") as bench:
            out.write("#include <lamp.h>\n")
            for line in bench:
                out.write(self.preprocess_line(line))

    def preprocess_line(self, line):
        def match_nondet():
            return "__VERIFIER_nondet_" in line

        def match_nondet_float() -> bool:
            nondets = ["__VERIFIER_nondet_float", "__VERIFIER_nondet_double"]
            return any(nondet in line for nondet in nondets)

        def match_float() -> bool:
            floats = ["float", "double"]
            return any(float_ in line for float_ in floats)

        def match_pointer() -> bool:
            return "__VERIFIER_nondet_pointer" in line

        def match_pthread() -> bool:
            return "pthread_" in line

        def match_globals() -> bool:
            extern = re.compile("extern[^(]*;")
            return extern.match(line) is not None

        def match_libm() -> bool:
            return '<math.h>' in line

        self.cfg.symbolic |= match_nondet()

        self.cfg.libm |= match_libm()

        if self.cfg.symbolic:
            self.cfg.floats |= match_nondet_float()
            self.cfg.pointer |= match_pointer()

        self.cfg.sequential &= not match_pthread()

        if match_globals():
            assert False, "unimplemented"
        
        return line

    def run(self):
        os.environ['DFSAN_OPTIONS'] = 'warn_unimplemented=0'
        os.environ['LART_ERROR_BACKTRACE'] = 'ON'
        
        self.preprocess()

        cc = compilation(self.cfg, self.preprocesed)
        ccout, ccerr, abstracted = cc.run()

        if not os.path.isfile(abstracted):     
            print("output:")
            with open(ccout, 'r') as out:
                print(out.read())

            print("error:")
            with open(ccerr, 'r') as err:
                print(err.read())
            ## TODO unknown result
            sys.exit()

        # FIXME: if error empty

        ver = verifier(self.cfg, abstracted)
        vout, verr = ver.run()

        # TODO: check return code of
        return analysis_result(verr, self.cfg)
