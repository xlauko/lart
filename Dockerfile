ARG CMAKE_VERSION=3.21
ARG LLVM_VERSION=12.0.1
ARG SVF_VERSION=2.2
ARG BUILD_BASE=ubuntu:rolling

#
# Base layer
#
FROM ${BUILD_BASE} as base

ARG CMAKE_VERSION

# Set for all apt-get install, must be at the very beginning of the Dockerfile.
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update
RUN apt-get install -y \
        bash git ninja-build ccache curl wget \
	build-essential clang zip unzip tar llvm-12 \
	pkg-config zlib1g-dev cmake -y

RUN apt-get autoremove -y
RUN apt-get clean all

ENV LLVM_DIR="/usr/lib/llvm-12/"

RUN rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src
RUN mkdir -p /usr/opt/cmake
RUN wget http://www.cmake.org/files/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.0-linux-x86_64.sh
RUN sh ./cmake-${CMAKE_VERSION}.0-linux-x86_64.sh --prefix=/usr/opt/cmake --skip-license
RUN rm ./cmake-${CMAKE_VERSION}.0-linux-x86_64.sh
ENV PATH="/usr/opt/cmake/bin:${PATH}"

#
# LIBCXX build for LART runtime
#
FROM base as dataflow-libs

WORKDIR /usr/src
ARG LLVM_VERSION

RUN curl -sSL https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-${LLVM_VERSION}.tar.gz \
        | tar -xvz

RUN mv /usr/src/llvm-project-llvmorg-${LLVM_VERSION} /usr/src/llvm

ENV LIBCXX_DATAFLOW_DIR="/usr/opt/libcxx-dataflow/"

WORKDIR /usr/src/llvm

RUN cmake -GNinja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_USE_SANITIZER="DataFlow" \
  -DCMAKE_INSTALL_PREFIX:PATH=${LIBCXX_DATAFLOW_DIR} \
  -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
  -S llvm \
  -B build-dataflow

RUN cmake --build build-dataflow --target cxx cxxabi -- -j 4
RUN cmake --build build-dataflow --target install-cxx install-cxxabi

FROM dataflow-libs as svf

ENV SVF_INSTALL_DIR "/usr/opt/svf/"
ARG SVF_VERSION

WORKDIR /usr/src/

RUN curl -sSL https://github.com/SVF-tools/SVF/archive/refs/tags/SVF-${SVF_VERSION}.tar.gz \
    | tar -xvz
RUN mv SVF-SVF-${SVF_VERSION} svf

WORKDIR /usr/src/svf

RUN cmake \
        -GNinja \
	-DLLVM_DIR=${LLVM_DIR} \
        -DCMAKE_INSTALL_PREFIX=${SVF_INSTALL_DIR} \
        -B build \
        -S .

RUN cmake --build build
RUN cmake --build build --target install

FROM svf as deps

# # ARM processor compability
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

WORKDIR /usr/opt/

RUN git clone https://github.com/microsoft/vcpkg \
        && ./vcpkg/bootstrap-vcpkg.sh

RUN ./vcpkg/vcpkg update
RUN ./vcpkg/vcpkg install spdlog catch2

ENV VCPKG_TOOLCHAIN "/usr/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"

FROM deps as lart-config

COPY . /usr/src/lart
WORKDIR /usr/src/lart

ENV LART_INSTALL_DIR "/usr/opt/lart"

RUN cmake \
    -GNinja \
    -DBUILD_LART_CC=ON \
    -DLLVM_INSTALL_DIR=${LLVM_DIR}/build/ \
    -DSVF_INSTALL_DIR=${SVF_INSTALL_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
    -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
    -B build-lartcc \
    -S .

FROM lart-config as lartcc

RUN cmake --build build-lartcc
RUN cmake --build build-lartcc --target install

FROM lartcc as runtime-config

RUN cmake \
    -GNinja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DBUILD_LART_RUNTIME=ON \
    -DLIBCXX_INSTALL_DIR=${LIBCXX_DATAFLOW_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
    -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
    -B build-runtime \
    -S .

FROM runtime-config as runtime

RUN cmake --build build-runtime
RUN cmake --build build-runtime --target install
# RUN cmake --build build-runtime

# RUN cmake --build build-runtime
