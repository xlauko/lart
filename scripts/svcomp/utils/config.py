#!/usr/bin/env python3

import tempfile
from typing import List, Optional

from . properties import property
from . log import error, logger


def unsupported(prp: property) -> None:
    error(f"property {prp} not supported yet")


class config:
    def __init__(self, lartcc, benchmark, properties, property_strings, architecture, lamp, workdir):
        # path to lartcc binary
        self.lartcc : str = lartcc

        self.workdir : str = workdir if workdir else tempfile.mkdtemp()
        self.tmpworkdir : bool = workdir is None

        # path to benchmark
        self.benchmark : str = benchmark

        # property setup
        self.properties : List[property] = properties
        self.property_strings : List[str] = property_strings

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

        self.print_cause : bool = False
        
        # file offset for line debug info
        self.file_offset = 0

        self.setup_property_options()

    def setup_property_options(self):
        supported_list = [property.reach]

        for prp in self.properties:
            if prp not in supported_list:
                unsupported(prp)
            if prp == property.memsafety:
                self.leakcheck = True
