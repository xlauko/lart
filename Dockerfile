ARG CMAKE_VERSION=3.21
ARG LLVM_VERSION=12.0.1
ARG SVF_VERSION=2.2
ARG BUILD_BASE=ubuntu:rolling

FROM ${BUILD_BASE} as base

ARG CMAKE_VERSION

# Set for all apt-get install, must be at the very beginning of the Dockerfile.
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update
RUN apt-get install bash sudo git ninja-build ccache curl \
	build-essential clang zip unzip tar zlib1g-dev \
	llvm-dev libssl-dev libc++ libc++-dev libtbb-dev \
	libc++abi-dev pkg-config gpg wget -y

# Set clang as default C and C++ compiler.
ENV CC=/usr/bin/clang-12
ENV CXX=/usr/bin/clang++-12

WORKDIR /usr/src/

# Build CMake.
# TODO Remove
RUN curl -SL http://www.cmake.org/files/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.0.tar.gz \
        | tar -xzC .

RUN mv cmake-${CMAKE_VERSION}.0 cmake
RUN cd cmake && ./configure && make install

FROM base as llvm

ARG LLVM_VERSION
RUN curl -SL https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/llvm-project-${LLVM_VERSION}.src.tar.xz \
        | tar -xJC .

RUN mv /usr/src/llvm-project-${LLVM_VERSION}.src /usr/src/llvm

FROM llvm as llvm_build

ENV LIBCXX_DATAFLOW_DIR="/usr/opt/libcxx-dataflow/"
ENV LLVM_DIR="/usr/opt/llvm/"

RUN mkdir -p /usr/build/libcxx
WORKDIR /usr/build/libcxx

RUN cmake -GNinja /usr/src/llvm/llvm \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_USE_SANITIZER="DataFlow" \
  -DLLVM_ENABLE_LIBCXX=ON \
  -DCMAKE_INSTALL_PREFIX:PATH=${LIBCXX_DATAFLOW_DIR} .. \
  -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi"

RUN cmake --build .
RUN cmake --build . --target install

RUN mkdir -p /usr/build/llvm
WORKDIR /usr/build/llvm

RUN cmake -GNinja /usr/src/llvm/llvm \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_ENABLE_LIBCXX=ON \
  -DCMAKE_INSTALL_PREFIX:PATH=${LLVM_DIR} .. \
  -DLLVM_ENABLE_PROJECTS="llvm"

RUN cmake --build .
RUN cmake --build . --target install

FROM llvm_build as svf_build
ARG SVF_VERSION

WORKDIR /usr/src/

RUN curl -SL https://github.com/SVF-tools/SVF/archive/refs/tags/SVF-${SVF_VERSION}.tar.gz \
        | tar -xzC .

RUN mv SVF-SVF-${SVF_VERSION} svf

WORKDIR /usr/src/svf

# ENV LLVM_DIR="/usr/lib/llvm-12"

RUN mkdir build && cd build \
        && cmake -DLLVM_DIR="/usr/lib/llvm-12" \
		 -DCMAKE_INSTALL_PREFIX:PATH="/usr/opt/svf/" .. \
        && cmake --build . --target install --config Release

FROM svf_build as deps

# ARM processor compability
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

WORKDIR /usr/opt/

RUN git clone https://github.com/microsoft/vcpkg \
        && ./vcpkg/bootstrap-vcpkg.sh

RUN ./vcpkg/vcpkg update
RUN ./vcpkg/vcpkg install range-v3 spdlog cppcoro catch2

FROM deps as lart_build

COPY . /usr/src/lart
WORKDIR /usr/src/lart

RUN cmake -DLIBCXX_INSTALL_DIR=${LIBCXX_DATAFLOW_DIR} \
      -DSVF_INSTALL_DIR="/usr/opt/svf/" \
      -DCMAKE_TOOLCHAIN_FILE="/usr/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" \
      -B build \
      -S .
