## Build on pontos

### Install LLVM

```bash
mkdir -p ~/src/libcxx
cd ~/src/libcxx
git clone https://github.com/llvm/llvm-project.git --depth 1 .

export LIBCXX_DATAFLOW_DIR=~/opt/libcxx-dataflow/
 
cmake -GNinja \
      -DCMAKE_C_COMPILER=clang-12 \
      -DCMAKE_CXX_COMPILER=clang++-12 \
      -DLLVM_USE_SANITIZER="DataFlow" \
      -DCMAKE_INSTALL_PREFIX=${LIBCXX_DATAFLOW_DIR} \
      -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
      -S llvm \
      -B build-dataflow
  
cmake --build build-dataflow --target cxx cxxabi
cmake --build build-dataflow --target install-cxx install-cxxabi
```


### Install SVF

```bash
mkdir -p ~/src/svf
cd ~/src/svf
git clone https://github.com/SVF-tools/SVF --depth 1 -b SVF-2.3 .

export LLVM_INSTALL_DIR=/usr/lib/llvm-12/
export SVF_INSTALL_DIR=~/opt/svf/

cmake -GNinja \
      -DLLVM_DIR="${LLVM_INSTALL_DIR}" \
      -DCMAKE_INSTALL_PREFIX="${SVF_INSTALL_DIR}" \
      -B build \
      -S .

cmake --build build
cmake --build build --target install
```


### Install vcpkg and packages

```bash
mkdir -p ~/opt/vcpkg
cd ~/opt/vcpkg
git clone https://github.com/microsoft/vcpkg.git . && ./bootstrap-vcpkg.sh

export PATH=~/opt/vcpkg:${PATH}
export VCPKG_TOOLCHAIN=~/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

vcpkg update
vcpkg install spdlog catch2 doctest
```


### Build-lart

```bash
mkdir -p ~/src/lart
cd ~/src/lart
git clone --recurse-submodules https://github.com/xlauko/lart .

export LART_INSTALL_DIR=~/opt/lart/

cmake -GNinja \
      -DBUILD_LART_CC=ON \
      -DENABLE_TESTING=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DLLVM_INSTALL_DIR=${LLVM_INSTALL_DIR}/build/ \
      -DSVF_INSTALL_DIR=${SVF_INSTALL_DIR} \
      -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
      -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
      -B build-lart \
      -S .

cmake --build build-lart
cmake --build build-lart --target install
```


### Build-runtime

```bash
cd ~/src/lart

cmake -GNinja \
      -DCMAKE_C_COMPILER=clang-12 \
      -DCMAKE_CXX_COMPILER=clang++-12 \
      -DBUILD_LART_RUNTIME=ON \
      -DLLVM_INSTALL_DIR=${LLVM_INSTALL_DIR} \
      -DLIBCXX_INSTALL_DIR=${LIBCXX_DATAFLOW_DIR} \
      -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
      -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
      -B build-runtime \
      -S .

cmake --build build-runtime
cmake --build build-runtime --target install
```


### Add to `.bashrc`

```bash
export PATH="${HOME}/opt/lart/bin/lartcc:${PATH}"
```


### Example usage

```bash
lartcc unit ./test/basic/any-a.c -I./test/include/ -o abstract
./abstract
```


### Install `lit` & run tests

```bash
pip install --user lit
export PATH=$PATH:~/.local/bin

lit -v build-lart/test/ --filter=basic
```
