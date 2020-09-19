#!/bin/bash

LLVM_VERSION=$1

# # Setup LLVM
curl -SL https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04.tar.xz \
        | tar -xJC . && \
mv clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04 /usr/src/llvm && \
echo 'export PATH=/usr/src/llvm/bin:$PATH' >> ~/.bashrc && \
echo 'export LD_LIBRARY_PATH=/usr/src/llvm/lib:$LD_LIBRARY_PATH' >> ~/.bashrc

