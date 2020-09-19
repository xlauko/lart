# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
FROM gcc:latest

ENV LLVM_INSTALL_DIR="/usr/src/llvm"

ARG LLVM_VERSION=10.0.0

# Set for all apt-get install, must be at the very beginning of the Dockerfile.
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update && apt-get install bash sudo -y

WORKDIR /usr/src/lart

RUN mkdir -p /usr/src/lart/scripts

COPY ./scripts/install-deps.sh /usr/src/lart/scripts/
RUN ./scripts/install-deps.sh

# Install CMake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-Linux-x86_64.sh \
    -q -O /tmp/cmake-install.sh \
    && chmod u+x /tmp/cmake-install.sh \
    && mkdir /usr/bin/cmake \
    && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
    && rm /tmp/cmake-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

# Non-interactive modes get set back.
ENV DEBIAN_FRONTEND newtq

ENV CMAKE_C_COMPILER=/usr/bin/gcc
ENV CMAKE_CXX_COMPILER=/usr/bin/g++

COPY ./scripts/install-llvm.sh /usr/src/lart/scripts/
RUN ./scripts/install-llvm.sh ${LLVM_VERSION}

COPY . /usr/src/lart

RUN ls /usr/src/

# This command compiles your app using GCC, adjust for your source code
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_INSTALL_DIR="/usr/src/llvm"

cmake --build build

LABEL Name=lart Version=0.1.0
