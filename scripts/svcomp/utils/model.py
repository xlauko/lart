#!/usr/bin/env python3

import ctypes
from . log import logger

class ModelVar:
    def __init__(self, call, line, value):
        self.call = call
        self.line = line
        self.value = value

    def __str__(self):
        return f"var: {self.call}:{self.line} = {self.value}"

class Model:
    def __init__(self):
        self.vars = []

    def parse(self, report, line_offset):
        nondet_calls = []
        model = dict()

        for line in report:
            if line.startswith("[lamp any]"):
                nondet_calls.append(self.parse_nondet_call(line))    
            if line.startswith("[term model]"):
                name, value = self.parse_term_var(line)
                index = self.parse_index(name) - 1
                model[index] = value

        for idx, call in enumerate(nondet_calls):
            value = self.parse_value(call[0], model.get(idx, "0"))
            line = str(int(call[2]) - line_offset)
            var = ModelVar(call[0], line, value)
            logger().info(f"{var}")
            self.vars.append(var)
        

    def parse_index(self, var_name):
        return int(var_name.split('_')[1])

    def parse_nondet_call(self, line):
        name = line.lstrip("[lamp any]").strip()
        parsed = name.split(":")
        return parsed

    def parse_term_var(self, line):
        line = line.lstrip("[term model]")
        parts = line.split(" = ")
        return parts[0].strip(), parts[1].strip()

    def parse_value(self, nondet, parsed):
        if "pointer" in nondet:
            return None # FIXME pointer models

        try:
            if "float" in nondet:
                val = float(parsed)
                return ctypes.c_float(val)
            if "double" in nondet:
                val = float(parsed)
                return ctypes.c_double(val)
        except ValueError:
            return None

        try:
            # integer models:
            parsed = parsed.lstrip("#x")
            val = int(parsed, 16)
            bw = val.bit_length()
            if "uint" in nondet and bw <= 32:
                return ctypes.c_uint(val)
            if "unsigned" in nondet and bw <= 32:
                return ctypes.c_uint(val)
            if "int" in nondet and bw <= 32:
                return ctypes.c_int(val)
            if "bool" in nondet and bw <= 8:
                return ctypes.c_bool(val)
            if "char" in nondet and bw <= 8:
                return ctypes.c_byte(val)
            if "ushort" in nondet and bw <= 16:
                return ctypes.c_ushort(val)
            if "short" in nondet and bw <= 16:
                return ctypes.c_short(val)
            if "ulong" in nondet and bw <= 64:
                return ctypes.c_ulong(val)
            if "long" in nondet and bw <= 64:
                return ctypes.c_long(val)
        except ValueError:
            return None
        return None

