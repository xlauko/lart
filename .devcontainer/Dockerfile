# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.245.2/containers/cpp/.devcontainer/base.Dockerfile

ARG VARIANT="ubuntu-22.04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT} as base

#
# setup cmake
#
ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.24.2"

RUN chmod +x ./reinstall-cmake.sh && ./reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}

#
# install deps
#
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update \
    && apt-get -y install --no-install-recommends \
        software-properties-common python3-pip ccache cmake lld \
        llvm-13 clang-13 libz3-dev z3 zip unzip tar llvm-13-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/

ENV LLVM_INSTALL_DIR="/usr/lib/llvm-13/"
ENV Z3_DIR "/usr/opt/z3-4.11.2-x64-glibc-2.31/"
RUN mkdir -p "/usr/opt/"

RUN wget -c "https://github.com/Z3Prover/z3/releases/download/z3-4.11.2/z3-4.11.2-x64-glibc-2.31.zip" -O z3.zip
RUN unzip -q z3.zip -d /usr/opt
RUN rm z3.zip
