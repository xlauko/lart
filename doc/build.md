## Build Against LLVM 10


## Dataflow sanitized libcxxabi

```
mkdir build-libcxxabi
cd build-cxxabi

cmake -G "Ninja" \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_INSTALL_PREFIX=/path/to/install \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS=-fsanitize=dataflow \
  -DCMAKE_CXX_FLAGS=-fsanitize=dataflow \
  -DLLVM_PATH=../ \
  -DLIBCXXABI_ENABLE_SHARED=NO \
  -DLIBCXXABI_LIBCXX_PATH=../libcxx \
  ../libcxxabi

cmake --build .
cmake --install .
```

## Dataflow sanitized libcxx

```
mkdir build-libcxx
cd build-cxx

cmake -G "Ninja" \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_INSTALL_PREFIX=/path/to/install \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS=-fsanitize=dataflow \
  -DCMAKE_CXX_FLAGS=-fsanitize=dataflow \
  -DLLVM_PATH=../ \
  -DLIBCXX_ENABLE_SHARED=OFF \
  -DLIBCXX_CXX_ABI=libcxxabi \
  -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
  -DLIBCXX_CXX_ABI_INCLUDE_PATHS=../libcxxabi/include/ \
  -DLIBCXX_CXX_ABI_LIBRARY_PATH=../build-libcxxabi/lib/ \
  ../libcxx

cmake --build .
cmake --install .
```

Set cmake variable `LIBCXX_INSTALL_DIR` to `/path/to/install`.