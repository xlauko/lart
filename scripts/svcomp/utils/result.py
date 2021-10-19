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
    def __init__(self, report, cfg):
        self.cfg = cfg
        self.model = Model()

        self.verification_result = self.process_report(report) if report else result.unknown

        if not self.valid_result():
            logger().info(f"invalid result: {self.verification_result}")
            self.verification_result = result.unknown

        logger().info(f"result: {self.verification_result}")
        logger().info(f"properties: {self.cfg.properties}")
        if get_result_class(self.verification_result) is result_class.false:
            # self.backtrace = self.parse_backtrace(report)
            if self.cfg.symbolic:
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
        return False

    def process_lines(self, report):
        bounded = False
        fatal = False
        for line in report:
            logger().info(f"report line: {line}")
            if self.ignore_result(line):
                return result.unknown
            if line.startswith("[lart fault]"):
                if "reach_error" in line:
                    return result.false_reach
                return result.unknown
            if line.startswith("[lart status]"):
                if "bounded exit" in line:
                    bounded = True
                return result.unknown
            if "FATAL: DataFlowSanitizer" in line:
                fatal = True
        if bounded or fatal:
            return result.unknown
        return result.true

    def process_report(self, report_path):
        logger().info(f"processing report: {report_path}")

        try:
            with open(report_path, "r") as report:
                return self.process_lines(report)
        except EnvironmentError:
            return result.unknown

    # def parse_backtrace(self, report_path):
    #     logger().info(f"parsing backtrace: {report_path}")
    #     backtrace = []

    #     with open(report_path, "r") as report:
    #         prefix = "[lart backtrace]"
    #         for line in report:
    #             if line.startswith(prefix):
    #                 logger().info(line)
    #                 name = line.lstrip(prefix).strip()
    #                 backtrace.append(name)

    #     backtrace.reverse()

    #     logger().info(f"backtrace {backtrace}")
    #     return backtrace

    def parse_model(self, report_path):
        logger().info(f"parsing model: {report_path}")
        model = Model()
        with open(report_path, "r") as report:
            model.parse(report, self.cfg.file_offset)
        return model


    def can_generate_witness(self):
        cls = get_result_class(self.verification_result)
        return cls == result_class.true or cls == result_class.true 

    def generate_witness(self, path: str):
        assert path

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
