# jit++

<!-- badges:start -->
[![build](https://img.shields.io/github/actions/workflow/status/ekmett/jitplusplus/build.yml?branch=main&style=flat&label=build&logo=githubactions&logoColor=white)](https://github.com/ekmett/jitplusplus/actions/workflows/build.yml?query=branch%3Amain)
[![issues](https://img.shields.io/github/issues/ekmett/jitplusplus?style=flat&label=issues&color=007ec6&logo=github&logoColor=white)](https://github.com/ekmett/jitplusplus/issues)
[![commits](https://img.shields.io/github/commit-activity/w/ekmett/jitplusplus?style=flat&label=commits&color=007ec6&logo=github&logoColor=white)](https://github.com/ekmett/jitplusplus/activity)

[![CMake: 3.30+](https://img.shields.io/static/v1?label=CMake&message=3.30%2B&color=064F8C&style=flat&logo=cmake&logoColor=white)](CMakeLists.txt)
[![C++: 26](https://img.shields.io/static/v1?label=C%2B%2B&message=26&color=00599C&style=flat&logo=cplusplus&logoColor=white)](README.md)
[![Clang: 19](https://img.shields.io/static/v1?label=Clang&message=19&color=6f42c1&style=flat&logo=llvm&logoColor=white)](README.md)

[![OS: Linux](https://img.shields.io/static/v1?label=OS&message=Linux&color=64748b&style=flat)](CMakeLists.txt)
[![CPU: x86-64](https://img.shields.io/static/v1?label=CPU&message=x86-64&color=64748b&style=flat)](CMakeLists.txt)

[![license: BSD-2-Clause OR Apache-2.0](assets/badges/license.svg)](https://github.com/ekmett/jitplusplus/blob/de293ad7c1abc07441add9342de8ca7fccc27afd/COPYING)
[![Contributor Covenant: 2.0](https://img.shields.io/static/v1?label=Contributor+Covenant&message=2.0&color=007ec6&style=flat&logo=contributorcovenant&logoColor=white)](CODE_OF_CONDUCT.md)

[![docs: read](https://img.shields.io/static/v1?label=docs&message=read&color=007ec6&style=flat)](docs/building.md)
<!-- badges:end -->

An experimental cooperative x86-64 interpreter. A call to `start()` captures
machine state and begins interpreting the caller's instructions. `stop()` or an
unsupported instruction returns execution to native code. The long-term aim is
a tracing JIT; the current implementation establishes the native/interpreted/native
transition.

The C++26 module `jitpp` exports `jitpp::application` and `jitpp::interpreter`.
The interpreter owns registers, decoding, flags, and instruction execution in
one concrete class, with no inheritance, virtual dispatch, or PIMPL allocation.
Its state and method bodies live together in [interpreter.ccm](jit++/interpreter.ccm);
[application.ccm](jit++/application.ccm) contains runtime setup.

## Build and use

The tested toolchain is Clang 19, CMake 3.30 and Ninja on Linux x86-64.
Install glog, gflags and udis86 using the [build guide](docs/building.md), then:

```sh
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++-19 \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/build-deps"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

An enclosing CMake project can consume the module with:

```cmake
add_subdirectory(path/to/jitplusplus)
add_executable(example main.cc)
set_target_properties(example PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(example PRIVATE jitpp::jitpp)
```

Application code uses `import jitpp;`. CMake propagates the C++26 requirement;
producer and consumer must use the same compiler and compatible language/runtime
settings. Exceptions remain enabled because unsupported instructions use them
to return to native execution.

## Interpreter and validation

The [source guide](jit++/README.md) describes module ownership, the assembly
boundary, and the decode/execute loop. The [decoder guide](docs/decoding.md)
explains the encoding table and its current limitations.

The round-trip test checks arithmetic, stores, flags on native resumption,
repeated entry, step-limited exit, and unsupported-instruction fallback. The
original demo and a small dispatch benchmark also build through `import jitpp;`.
See the [build guide](docs/building.md#tests-and-benchmark) for commands.

Instruction coverage, syscall/TLS behavior, and extended vector state remain
incomplete. The runtime uses a worker pthread and a fixed gap on the caller's
stack. These checks establish a working foundation, not general x86-64 correctness.

## License

Copyright (c) 2008 Edward Kmett. All rights reserved. See [COPYING](COPYING).
