 cmake \
    -GNinja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DBUILD_LART_RUNTIME=ON \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DLLVM_INSTALL_DIR=${LLVM_INSTALL_DIR} \
    -DLIBCXX_INSTALL_DIR=${LIBCXX_DATAFLOW_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_TOOLCHAIN} \
    -DCMAKE_INSTALL_PREFIX=${LART_INSTALL_DIR} \
    -B build-runtime \
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