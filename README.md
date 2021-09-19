# LART: LLVM Abstraction and Refinement Tool

[![Build Status](https://travis-ci.com/xlauko/lart.svg?branch=master)](https://travis-ci.com/xlauko/lart)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

LART = LLVM Abstraction & Refinement Tool. The goal of this tool is to provide LLVM-to-LLVM transformations that implement various program abstractions. In terms of the instruction set, the resulting programs are normal, concrete LLVM programs that can be executed and analyzed. Extra information about the abstraction(s) in effect over a (fragment) program is inserted using special LLVM intrinsic functions and LLVM metadata nodes.
LART provides both a standalone tool that processes on-disk bitcode files, as well as a framework that can be integrated into complex LLVM-based tools. The primary motivation behind LART is to provide a "preprocessor" for LLVM-based model checkers and other analysis tools, simplifying their job by reducing the problem size without compromising the soundness of the analyses.
The abstractions implemented by LART can be usually refined based on specific instructions about which "part" of the abstraction is too rough (an abstraction that is too rough will create spurious errors visible to subsequent analyses but not present in the original program).

## Abstractions for LLVM Bitcode

The purpose of the entire exercise is to abstract away information from LLVM bitcode, making subsequent analyses more efficient (at the expense of some precision). To this end, we mainly need to be able to encode non-deterministic choice in LLVM programs, which can be done simply through a special-purpose function (similar to LLVM intrinsics). The function is named `@lart.choice`, takes a pair of bounds as arguments and non-deterministically returns a value that falls between those bounds.

This extension to LLVM semantics needs to be recognized by the downstream tool. This is also the only crucial deviation from standard LLVM bitcode. Many analysis tools will already implement a similar mechanism, either internally or even with an external interface. Adapting tools without support for `@lart.choice` to work with LART is usually very straightforward.

There are other special-purpose functions provided by LART, namely the `@lart.meta.*` family, but as far as these instructions are concerned, most tools will be able to safely ignore their existence, just like with existing `@llvm.dbg.*` calls. Program transformations would be expected to retain those calls in case LART is called in to refine an abstraction (each abstraction provided by LART comes with a corresponding refinement procedure, which will often need to find the `@lart.meta` calls inserted by the abstraction).

While most traditional abstraction engines work as interpreters, abstractions can also be "compiled" into programs. Instead of (re-)interpreting instructions symbolically, the symbolic instructions can be compiled. In case of predicate abstraction, the resulting bitcode will directly manipulate and use predicate valuations instead of concrete variables. As explained above, the important difference is that the bitcode needs to make non-deterministic choices, since some predicates may have indeterminate valuations (are both true and false). Some variables could be even abstracted away entirely, and all tests on such variables will yield both yes and no answers.

## Build

```
mkdir build

cmake -DLLVM_INSTALL_DIR=<path to llvm to use> \
      -DLIBCXX_INSTALL_DIR=<path to sanitized libcxx> \
      -B build \
      -S .

cmake --build build
```

In docker use:

```
./scripts/build.sh
```

## Run Compiler

```
./build/bin/lartcc <domain> <compiler arguments> in.c
```

## OPT

```
opt -load build/lib/cc/liblart_module.so -lart < in.bc > out.bc
```

## Test

```
lit -v build/test
```

Note: `build/lartcc/lartcc` has to have exacutabple permisions.