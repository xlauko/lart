# LART: LLVM Abstraction and Refinement Tool

[![Build Status](https://travis-ci.com/xlauko/lart.svg?branch=master)](https://travis-ci.com/xlauko/lart)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Build

## Run Compiler

```
./build/lib/cc/lartcc in.c
```

## OPT

```
opt -load build/lib/cc/libcc.so -lart < in.bc > out.bc
```
