#!/usr/bin/env python3

from enum import Enum

from . log import error

class property(Enum):
    """
    Property names based on https://sv-comp.sosy-lab.org/2022/rules.php
    """

    reach       = 'unreach-call'
    termination = 'termination'
    overflow    = 'no-overflow'
    deadlock    = 'no-deadlock'
    defbehavior = 'def-behavior'
    deref       = 'valid-deref'
    free        = 'valid-free'
    memtrack    = 'valid-memtrack'
    memcleanup  = 'valid-memcleanup'
    memsafety   = 'valid-memsafety' # internal meta property

    def is_memsafety(self):
        return self in [ self.deref, self.free, self.memtrack, self.memsafety ]

    def __str__(self):
        return self.value

    @staticmethod
    def from_string(s):
        try:
            return config[s]
        except KeyError:
            error("unknown property")

"""
This maps content of property files to property name
"""
property_names = {
    'LTL(G ! call(reach_error()))'     : property.reach,
    'LTL(F end)'                       : property.termination,
    'LTL(G valid-free)'                : property.free,
    'LTL(G valid-deref)'               : property.deref,
    'LTL(G valid-memtrack)'            : property.memtrack,
    'LTL(G valid-memcleanup)'          : property.memcleanup,
    'LTL(G ! overflow)'                : property.overflow,
    'LTL(G ! deadlock)'                : property.deadlock,
    'LTL(G def-behavior)'              : property.defbehavior
}
