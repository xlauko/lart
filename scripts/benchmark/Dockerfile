# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.245.2/containers/cpp/.devcontainer/base.Dockerfile

ARG VARIANT="ubuntu-22.04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

#
# install deps
#
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update \
    && apt-get -y install --no-install-recommends \
        software-properties-common python3-pip ccache cmake lld \
        llvm-13 clang-13 libz3-dev z3 zip unzip tar llvm-13-dev \
        ca-certificates apt-transport-https stress git \
    && rm -rf /var/lib/apt/lists/*

#
# download lart
#
WORKDIR /usr/src/
RUN git clone --recurse-submodules -j8 https://github.com/xlauko/lart.git

#
# setup cmake
#

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.24.2"

RUN chmod +x /usr/src/lart/.devcontainer/reinstall-cmake.sh \
    && /usr/src/lart/.devcontainer/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}

WORKDIR /usr/src/

ENV LLVM_INSTALL_DIR="/usr/lib/llvm-13/"
ENV Z3_DIR "/usr/opt/z3-4.11.2-x64-glibc-2.31/"
RUN mkdir -p "/usr/opt/"

RUN wget -c "https://github.com/Z3Prover/z3/releases/download/z3-4.11.2/z3-4.11.2-x64-glibc-2.31.zip" -O z3.zip
RUN unzip -q z3.zip -d /usr/opt
RUN rm z3.zip

#
# build lart
#
WORKDIR /usr/src/lart

RUN cmake --preset ninja-surveyor \
    -DCMAKE_C_COMPILER=clang-14 \
    -DCMAKE_CXX_COMPILER=clang++-14

RUN cmake --build ./builds/ninja-surveyor

ENV PATH="/usr/src/lart/scripts/benchmark:${PATH}"
ENV PATH="/usr/src/lart/builds/ninja-surveyor/lartcc:${PATH}"
