# jit++

An experimental cooperative x86-64 interpreter. A call to `start()` captures
machine state and begins interpreting the caller's instructions. `stop()` or an
unsupported instruction returns execution to native code. The long-term aim is
a tracing JIT; the current implementation establishes the native/interpreted/native
transition.

The C++26 module `jitpp` exports `jitpp::application` and `jitpp::interpreter`.
The interpreter owns registers, decoding, flags, and instruction execution in
one concrete class, with no inheritance, virtual dispatch, or PIMPL allocation.

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
