#!/usr/bin/env python3

import tempfile
from typing import List, Optional

from . properties import property
from . log import error, logger


def unsupported(prp: property) -> None:
    error(f"property {prp} not supported yet")


class config:
    def __init__(self, lartcc, benchmark, properties, architecture, lamp, workdir):
        # path to lartcc binary
        self.lartcc : str = lartcc

        self.workdir : str = workdir if workdir else tempfile.mkdtemp()
        self.tmpworkdir : bool = workdir is None

        # path to benchmark
        self.benchmark : str = benchmark

        # property setup
        self.properties : List[property] = properties

        # architecture
        self.architecture = architecture

        # lart options config
        self.symbolic   : bool = False
        self.leakcheck  : bool = False
        self.floats     : bool = False
        self.pointer    : bool = False
        self.globals    : bool = False
        self.sequential : bool = True
        self.witness    : bool = True
        self.report     : Optional[str] = None
        self.lamp       : str = lamp
        
        # compiler options
        self.libm       : bool = False

        self.setup_property_options()

    def setup_property_options(self):
        unsupported_list = [
            property.memcleanup,
            property.termination,
            property.defbehavior,
            property.overflow
        ]

        for prp in self.properties:
            if prp in unsupported_list:
                unsupported(prp)
            if prp == property.memsafety:
                self.leakcheck = True

    def get_options(self) -> List[str]:
        return []
