# Building and using jit++

The build uses CMake 3.30 or newer and Ninja. Clang 19 with its matching
`clang-scan-deps` is the tested compiler. The assembly trampoline currently
requires Linux x86-64.

## Dependencies

Install CMake 3.30 or newer and the compiler/runtime dependencies. On
Debian/Ubuntu releases providing Clang 19:

```sh
sudo apt-get install build-essential clang-19 clang-tools-19 ninja-build \
  libgoogle-glog-dev libgflags-dev
```

udis86 supplies diagnostic disassembly. If it is not installed, the helper builds
a pinned revision and fixes its Python 3 table generator:

```sh
sudo apt-get install autoconf automake libtool git python3
sh bin/build-udis86.sh "$PWD/build-deps"
```

Autotools is needed only by this upstream dependency; jit++ itself uses CMake.

## Configure and build

```sh
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++-19 \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/build-deps"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Use `-DCMAKE_BUILD_TYPE=Debug` for an unoptimized build. Both configurations run
in CI. CMake generates `jit++/config.h` inside the build directory. Use
`CMAKE_PREFIX_PATH` to locate dependencies in other prefixes; it is unnecessary
when all dependencies are already in the toolchain's search paths.

The target `jitpp::jitpp` provides one static library and the `jitpp` module.
Consume it with `add_subdirectory`, as shown in the [README](../README.md).
Set `CXX_EXTENSIONS OFF` on consumers to match the module's strict C++26 mode.
The API is defined directly in `.ccm` files; importers use no project headers. `JITPP_BUILD_TESTS` defaults
to enabled for a standalone build and disabled when included by another project.

## Tests and benchmark

`test_roundtrip` uses a short assembly sequence to check that instructions
actually run under interpretation and that native execution resumes with the
expected registers and flags. Its checks remain active in Release builds.
`test_interpreter` runs the original demonstration. `test_decoder` checks table
lengths against generated udis86 probes; see the [decoder guide](decoding.md) for
its coverage and limits. For instruction logging:

```sh
./build/test_interpreter --logtostderr --v=1
```

The optional benchmark excludes worker-thread startup and native entry/exit:

```sh
cmake --build build --target bench_interpreter
./build/bench_interpreter
```

It reports the median of nine runs of a million-iteration add/dec/jnz loop.
Use the same compiler, optimization flags and logging settings for comparisons.
