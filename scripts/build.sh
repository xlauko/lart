export PATH=$HOME/src/lart/build-lart/lartcc:$PATH

export LIBCXX_DATAFLOW_DIR=~/opt/libcxx-dataflow/
export LART_INSTALL_DIR=~/opt/lart
export SVF_INSTALL_DIR=~/opt/svf/
export LLVM_INSTALL_DIR=/usr/lib/llvm-12/
export VCPKG_TOOLCHAIN=~/opt/vcpkg/scripts/buildsystems/vcpkg.cmake


 cmake \
    -GNinja \
    -DCMAKE_C_COMPILER=clang-12 \
    -DCMAKE_CXX_COMPILER=clang++-12 \
    -DBUILD_LART_RUNTIME=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLLVM_INSTALL_DIR=${LLVM_INSTALL_DIR} \
    -DLIBCXX_INSTALL_DIR=${LIBCXX_DATAFLOW_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
    -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
    -B build-runtime \
    -DCMAKE_INSTALL_PREFIX=~/opt/lart \
    -S .

cmake --build build-runtime
cmake --build build-runtime --target install

cmake \
    -GNinja \
    -DBUILD_LART_CC=ON \
    -DENABLE_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_INSTALL_DIR=${LLVM_INSTALL_DIR}/build/ \
    -DSVF_INSTALL_DIR=${SVF_INSTALL_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
    -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
    -B build \
    -S .

cmake --build build
cmake --build build --target install