ARG LLVM_VERSION=12.0.1
ARG SVF_VERSION=2.2
ARG BUILD_BASE=ubuntu:rolling

FROM ${BUILD_BASE} as base

# Set for all apt-get install, must be at the very beginning of the Dockerfile.
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update
RUN apt-get install bash sudo git ninja-build ccache curl build-essential cmake clang -y

WORKDIR /usr/src/

FROM base as llvm

ARG LLVM_VERSION
RUN curl -SL https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/llvm-project-${LLVM_VERSION}.src.tar.xz \
        | tar -xJC .

RUN mv /usr/src/llvm-project-${LLVM_VERSION}.src /usr/src/llvm

FROM llvm as llvm_build

RUN mkdir -p /usr/opt/llvm
WORKDIR /usr/opt/llvm

RUN cmake -GNinja /usr/src/llvm/llvm \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_USE_SANITIZER="DataFlow" \
  -DLLVM_ENABLE_LIBCXX=ON \
  -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi"

RUN ninja cxx cxxabi

FROM llvm_build as svf_build
ARG SVF_VERSION

WORKDIR /usr/opt/

RUN curl -SL https://github.com/SVF-tools/SVF/archive/refs/tags/SVF-${SVF_VERSION}.tar.gz \
        | tar -xzC .
RUN mv SVF-SVF-${SVF_VERSION} svf

WORKDIR /usr/opt/svf
RUN ./build.sh
