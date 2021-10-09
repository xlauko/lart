#!/usr/bin/env python3

from enum import Enum
from . model import Model
from . properties import property
from . log import logger
from . witness import WitnessWriter

class result(Enum):
    """
    Possible run results

    If a result is not in this list, it is handled as result.error.
    """

    # tool terminated properly and true/false does not make sense
    done = 'done'
    # tool could not find out an answer due to incompleteness
    unknown = 'unknown'
    # tool could not complete due to an error
    error = 'ERROR'
    # property holds
    true = 'true'
    # property does not hold
    false = 'false'
    # reachability property violated
    false_reach = 'false({})'.format(property.reach)
    # termination property violated
    false_termination = 'false({})'.format(property.termination)
    # overflow property violated
    false_overflow = 'false({})'.format(property.overflow)
    # deadlock property violated
    false_deadlock = 'false({})'.format(property.deadlock)
    # valid-deref property violated
    false_deref = 'false({})'.format(property.deref)
    # valid-free property violated
    false_free = 'false({})'.format(property.free)
    # valid-memtrack property violated
    false_memtrack = 'false({})'.format(property.memtrack)
    # valid-memcleanup property violated
    false_memcleanup = 'false({})'.format(property.memcleanup)
    # property violated and witness confirmed
    witness_confirmed = 'witness confirmed'

    def __str__(self):
        return self.value

    @staticmethod
    def from_string(s):
        try:
            return result[s]
        except KeyError:
            error("unknown result type")


class result_class(Enum):
    """
    clasification of results
    """

    true    = 'true'
    false   = 'false'
    unknown = 'unknown'
    error   = 'error'

    def __str__(self):
        return self.value

    @staticmethod
    def from_string(s):
        try:
            return result_class[s]
        except KeyError:
            error("unknown result type")


def get_result_class(res: result) -> result_class:
    if res is result.error:
        return result_class.error
    if res is result.unknown:
        return result_class.unknown
    if res is result.true:
        return result_class.true
    return result_class.false

"""
Map a property to all possible results for it.
"""
valid_results_per_property = {
    property.reach      : {result.true, result.false, result.false_reach},
    property.deref      : {result.true, result.false, result.false_deref},
    property.free       : {result.true, result.false, result.false_free},
    property.memtrack   : {result.true, result.false, result.false_memtrack},
    property.memsafety  : {result.true, result.false, result.false_deref, result.false_free, result.false_memtrack},
    property.memcleanup : {result.true, result.false, result.false_memcleanup},
    property.overflow   : {result.true, result.false, result.false_overflow},
    property.deadlock   : {result.true, result.false, result.false_deadlock},
    property.termination: {result.true, result.false, result.false_termination},
}


class analysis_result:
    def __init__(self, execution_result, report, config):
        self.cfg = config
        self.model = Model()

        if execution_result != 0:
            self.verification_result = result.error
        else:
            self.verification_result = self.process_report(report)

        if not self.valid_result():
            logger().info(f"invalid result: {self.verification_result}")
            self.verification_result = result.unknown

        logger().info(f"result: {self.verification_result}")
        logger().info(f"properties: {self.cfg.properties}")
        if self.cfg.symbolic:
            if get_result_class(self.verification_result) is result_class.false:
                self.model = self.parse_model(report)

    def valid_result(self):
        if self.verification_result == result.error:
            return True
        if self.verification_result == result.unknown:
            return True
        for valid in map(lambda prp: valid_results_per_property[prp], self.cfg.properties):
            if self.verification_result in valid:
                return True
        return False

    def ignore_result(self, line):
        if "lart.stubs" in line:
            logger().info("ignoring error in lart.stubs")
            return True
        # TODO add ignore cases
        return False

    def process_lines(self, report):
        for line in report:
            if self.ignore_result(line):
                return result.unknown
            if "error found: no" in line:
                return result.true
            if "error found: yes" in line:
                return result.false
            # TODO type of error


    def backtrace(self, report):
        trace = []
        for line in report:
            if "- symbol:" in line:
                trace.append(line.split(':')[1].strip())
        return trace

    def errortrace(self, report):
        fatal = ""
        fault = ""
        for line in report:
            if "FAULT:" in line:
                fault = line.split(':')[1].strip()
            if "FATAL:" in line:
                fatal = line.split(':')[1].strip()
                return (fault, fatal)
        return fault, fatal

    def process_report(self, report_path):
        logger().info(f"processing report: {report_path}")

        res = result.unknown
        backtrace = []
        fault = ""
        fatal = ""
        try:
            with open(report_path, "r") as report:
                res = self.process_lines(report)
                if res == result.false:
                    fault, fatal = self.errortrace(report)
                    backtrace = self.backtrace(report)
        except EnvironmentError:
            res = result.unknown

        if res == result.false:
            if fatal == "not implemented in userspace":
                return result.unknown
            if fatal == "memory error in userspace":
                if "__vm_obj_free" in fault:
                    return result.false_free
                if "out of bounds" in fault:
                    return result.false_deref
                return result.unknown
            if fatal == "memory leak in userspace":
                if "heap" in fault:
                    return result.false_memtrack
            if "__VERIFIER_error" in backtrace:
                return result.false_reach
            if "reach_error" in backtrace:
                return result.false_reach
            return result.unknown
        return res

    def parse_model(self, report_path):
        logger().info(f"parsing model: {report_path}")
        model = Model()
        with open(report_path, "r") as report:
            model.parse(report)
        return model


    def generate_witness(self, path: str):
        assert path is not None

        correctness = get_result_class(self.verification_result) is result_class.true
        writer = WitnessWriter(self.verification_result
                              , self.cfg
                              , self.model
                              , path
                              , correctness)
        writer.write("witness.graphml")

    def dump(self):
        result = get_result_class(self.verification_result)
        print(f"result: {result}")
