# LART: LLVM Abstraction and Refinement Tool

## Build

## Run Compiler

```
./build/lib/cc/lartcc in.c
```

## OPT

```
opt -load build/lib/cc/libcc.so -lart < in.bc > out.bc
```
