# Building

How to configure and build the plugin, its presets, toolkit requirements and
options, and how its targets are put together.

The plugin needs a rexlib to link against. It finds one with `find_package`,
so build the submodule into a prefix and point `rexlib_ROOT` at it:

```
git submodule update --init
cmake -S external/rexlib -B build/rexlib -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=build/rexlib-install -DCMAKE_INSTALL_LIBDIR=lib \
  -DBUILD_TESTING=OFF
cmake --build build/rexlib --target install --parallel
cmake -S . -B build/cuda -Drexlib_ROOT=build/rexlib-install
cmake --build build/cuda --parallel
```

With CMake 3.21 or newer the presets do the same: `cmake --preset debug`,
`cmake --build --preset debug`, `ctest --preset debug`, which build into
`build/debug`. There are `release`, `sanitize` (the address and undefined
behaviour sanitizers) and `sanitize-thread` presets too; the two sanitizer
presets are not offered on Windows. The `sanitize` test preset sets
`ASAN_OPTIONS=protect_shadow_gap=0`, without which the address sanitizer and
the CUDA runtime fight over the same memory. The `sanitize-thread` one points
`TSAN_OPTIONS` at `tests/tsan.supp`, which leaves out the lock cycles that go
through trompeloeil's own lock: a mocked call holds it through its actions,
and a case that calls back into the code under test from one takes that
code's lock under it. What only one machine has, such
as `rexlib_ROOT`, the CUDA paths or a compiler cache, goes in a
`CMakeUserPresets.json`, which git ignores.

A CUDA toolkit of 11 or newer is required. Unless `CMAKE_CUDA_ARCHITECTURES`
is set, `rexlib_cuda_all_major_architectures` asks nvcc which architectures it
supports and keeps the oldest of each major family as SASS, plus the PTX of
the newest. That needs CUDA 11.1; an older toolkit needs the architectures set
by hand. CUDA is enabled with `enable_language` after that, not in
`project()`.

nvcc accepts only the host compilers its release supports, and the C library
headers of their time: a GCC or glibc newer than the toolkit fails as soon as
CUDA is enabled, before any source is compiled. `CUDAHOSTCXX` names another
host compiler, as the wheel builds do; a glibc too new needs a newer toolkit.

CMake 3.18 is the minimum. The plugin is C++20, and CMake requires that
standard rather than preferring it, so that nothing newer gets in unnoticed.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_CUDA_BUILD_TESTING` | `ON` while `BUILD_TESTING` is on, forced `OFF` otherwise | Builds the test suites |
| `REXLIB_CUDA_REGISTER_TESTS_PER_BINARY` | `OFF` | Registers one CTest entry per test executable instead of one per Catch2 case |
| `REXLIB_CUDA_INSTALL` | `ON` | Generates the install rules |
| `REXLIB_CUDA_REXLIB_VERSION` | `0.1` | The rexlib version `find_package` asks for |
| `REXLIB_CUDA_RUNTIME_RPATH` | empty | An extra RPATH entry reaching the CUDA runtime |

The sources are compiled once, into the object library `rexlib-cuda-objects`,
which both the plugin and the unitary tests are built from: a `MODULE` cannot
be linked against. Their usage requirements travel through the interface
target `rexlib-cuda-deps`, because `$<TARGET_OBJECTS:>` carries only the
objects. Device code is compiled separately and linked into the module, so a
kernel may be called across translation units.

Every directory holding sources carries a `CMakeLists.txt` naming them, and
descends into the directories below it. A new `.cpp` has to be named there
before it is built: the lists are explicit rather than globbed, so that
adding a file cannot go unnoticed by the build system.

The plugin installs through rexlib's `rexlib_install_plugin`, which owns where
a plugin goes and the RPATH back to the library.
