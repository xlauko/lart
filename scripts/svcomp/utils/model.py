#!/usr/bin/env python3

import ctypes

class ModelVar:
    def __init__(self, call, line, value):
        self.call = call
        self.line = line
        self.value = value

    def __str__(self):
        return f"var:{self.call}:{self.line} = {self.value}"

class Model:
    def __init__(self):
        self.vars = []

    def parse(self, report):
        model_start = False
        for line in report:
            if "nondet:" in line:
                self.vars.append(self.parse_nondet_call(line))
            if "model:" in line:
                model_start = True
            if line.startswith("var_") and model_start:
                idx = int(line.split(":")[0].split("_")[1]) - 1
                self.vars[idx].value = self.parse_value(line, idx)

    def parse_nondet_call(self, line):
        parsed = line.strip().split(":")
        return ModelVar(parsed[1], parsed[2], None)


    def parse_value(self, line, idx):
        nondet = self.vars[idx].call
        if "pointer" in nondet:
            return None # FIXME pointer models

        parsed = line.strip().split(":")[1]

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
            val = int(parsed)
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

