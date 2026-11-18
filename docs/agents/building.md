# Building

How the library, its Python binding and its CUDA plugin are configured, built
and tested, and what their options do.

## The library

The dependencies' sources are git submodules (see
[dependencies](dependencies.md)), so a clone needs them first:
`git clone --recurse-submodules`, or `git submodule update --init` in an
existing clone.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
```

With CMake 3.21 or newer the presets do the same: `cmake --preset debug`,
`cmake --build --preset debug`, `ctest --preset debug`, which build into
`build/debug`. There are `release`, `sanitize` (the address and undefined
behaviour sanitizers) and `sanitize-thread` presets too; the two sanitizer
presets are not offered on Windows; they set the sanitizers' options too,
so a test run behaves as CI's. `coverage`, on Linux only, adds coverage
instrumentation, registers the tests per binary and passes valgrind the
options CI uses, for `ctest --test-dir build/coverage -T MemCheck`. `docs`
configures for the Doxygen reference alone and builds `rexlib-docs`.
`lint` runs the static checks: `cmake --build --preset lint` compiles the
library through clang-tidy, and `ctest --preset lint` runs editorconfig-checker
and ruff; the `lint` group of `pyproject.toml` installs all three
(`uv sync --group lint`, or `pip install --group lint`). clang-tidy reports
what it finds in every header that is neither a system header nor in a
`third_party/`, so the lint build runs on the dependencies the provider
builds or on installed ones alike.
Every test preset fails when it finds no test, runs the tests in a random
order and gives each ten minutes. Settings of a machine of one's own, such as
a compiler cache, go in a `CMakeUserPresets.json`, which git ignores.

`REXLIB_WARNINGS_AS_ERRORS` makes a warning fail rexlib's own targets. The
headers of a dependency built with rexlib are not system headers, so its
warnings count too; with the dependencies installed (see
[dependencies.md](dependencies.md)) they are, and only rexlib's warnings
fail the build. CI builds this way.

CMake 3.18 is the minimum. The sources are written to C++14 and reach anything
newer through the `REXLIB_*` feature macros, so that a compiler without it
still builds them; CMake asks for C++17 but leaves
`CMAKE_CXX_STANDARD_REQUIRED` off, so the request is a preference. One
definition does not follow this, `parse_device_index` in
`src/core/hardware/device_index.cpp`, which uses `std::from_chars`. The
public headers do, and `REXLIB_CHECK_HEADERS` holds them to it.

`VERSION` holds the version of the library and the binding; the CUDA plugin
has its own in `plugins/cuda/VERSION`. CMake writes it into the public header
`rexlib/core/version_macros.h`, in `include/` of the build tree, and installs
it with the other headers; it is the one source file CMake writes.
`get_library_version()` returns the version of the library that is loaded.
Before 1.0 a minor version may break the ABI, so the SONAME is `0.<minor>`
(`librexlib.so.0.1`) and `find_package(rexlib)` accepts only the same minor;
from 1.0 both follow the major version. The library's file is named by its
SONAME, beside the namelink.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_INSTALL` | `ON` | Generates the install and export rules |
| `REXLIB_BUILD_TESTING` | `ON` while `BUILD_TESTING` is on, forced `OFF` otherwise | Builds the test suites |
| `REXLIB_BUILD_DOCS` | `OFF` | Adds the Doxygen target `rexlib-docs` |
| `REXLIB_DOCS_USE_README_AS_MAINPAGE` | `ON` | Uses `README.md` as Doxygen's main page |
| `REXLIB_ENABLE_COVERAGE` | `OFF` | Adds coverage instrumentation |
| `REXLIB_REGISTER_TESTS_PER_BINARY` | `OFF` | Registers one CTest entry per test executable instead of one per Catch2 case; see [tests](tests.md) |
| `REXLIB_CHECK_HEADERS` | `OFF`, `ON` in the presets | Compiles every public header on its own as C++14, 17, 20 and 23 |
| `REXLIB_WARNINGS_AS_ERRORS` | `OFF` | Makes a compiler warning fail rexlib's own targets (`-Werror`, `/WX`) |
| `REXLIB_CLANG_TIDY` | `OFF`, `ON` in `lint` | Runs clang-tidy on the library's own sources as they compile; a finding fails the build |
| `REXLIB_CHECK_STYLE` | `OFF`, `ON` in `lint` | Adds `rexlib-style`, a test running editorconfig-checker over every file git tracks, and `rexlib-ruff`, a test running ruff over the Python files `ruff.toml` names |
| `REXLIB_BUILD_PYTHON` | `OFF` | Adds the Python binding in `python/`, which needs an interpreter with nanobind; see [the binding](#the-binding) |
| `REXLIB_BUILD_BENCHMARKS` | `OFF`, and only with `REXLIB_BUILD_TESTING` | Builds `rexlib-benchmarks`, Catch2 benchmarks run by hand and never by CTest |

`REXLIB_BUILD_TESTING` lets a project that embeds rexlib drop its tests without
turning off `BUILD_TESTING` for its own.

With `REXLIB_INSTALL` on, the tests include four labelled `install`, chained by
fixtures. The first installs the build into a directory of the build tree. The
next two build `tests/install/plugin`, a plugin of its own, against that
install, and install it beside the library with `rexlib_install_plugin`. The
last builds `tests/install/consumer`, a C++14 project that finds rexlib with
`find_package` and nothing else, and runs it, on Windows with the installed
library's directory on `PATH`. The consumer loads the plugin from the default
plugin directory and runs its program on the device the plugin adds. A header
that is not installed, a package that does not describe the library, or a
plugin SDK that is not enough fails there.

`rexlib-docs` writes the documentation of the public headers under
`docs/doxygen/html` of the build tree. A warning of Doxygen fails it, so a
comment that names a parameter that is not there does not go unnoticed; a
member left undocumented is not a warning. Doxygen leaves out what sits in
a namespace it has no documentation for, so a new namespace is documented in
`docs/doxygen/namespaces.dox`. The macros that declare an operation are given
to Doxygen as the class they declare, in `docs/doxygen/CMakeLists.txt`, so
that the comment above a declaration documents its class.

With `REXLIB_DOCS_USE_README_AS_MAINPAGE=ON`, the main page is taken from
`README.md`, badges included, so the front page and the repository overview
stay in one place.

## The binding

The binding is part of rexlib's build, off by default. It needs an
interpreter with the `dev` group of `pyproject.toml` installed
(`pip install --group dev`, pip 25.1 or newer, or `uv sync`):

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DREXLIB_BUILD_PYTHON=ON \
  -DPython_EXECUTABLE=$(which python)
cmake --build build -j
ctest --test-dir build -L python
```

`ctest -L python` installs the build into a directory of the build tree and
runs the tests against that install; see [tests](tests.md).

Built alone, the binding finds an installed rexlib of the same version, as
a distribution that packages the two apart does:

```
cmake -S python -B build/python -Drexlib_ROOT=<prefix> \
  -DPython_EXECUTABLE=$(which python)
cmake --build build/python -j
cmake --install build/python --prefix <prefix>
```

`pip install .` builds a wheel of the whole repository with the binding on,
as `pyproject.toml` sets it up. That build is thrown away with its
directory, so installing twice compiles the binding twice.
`SKBUILD_BUILD_DIR` names one to keep and is left to whoever is installing
rather than set in `pyproject.toml`: the gain is a developer's repeated
install, while the cost, a CMake cache that outlives the options it was
configured with, would fall on every user and on every wheel job.

The package is installed into `REXLIB_PYTHON_INSTALL_DIR` under the install
prefix: by default the platform library directory that the interpreter's
install scheme names, such as `lib/python3.11/site-packages`. A wheel's
prefix is the package directory itself, with the library and its CMake
package in `lib/` on every platform. Its `cmake.root` entry point names the
package, so that a project built with scikit-build-core against the installed
wheel finds rexlib's CMake package with `find_package(rexlib)`.
`pyproject.toml` sets `CMAKE_INSTALL_LIBDIR` because GNUInstallDirs would
pick `lib64` in the manylinux containers, and CMake does not search `lib64`
for a package on Debian, Ubuntu or Arch, so a wheel built that way is not
found there. The build writes the path from the
package to the prefix into the extension's RPATH; built against an installed
rexlib, the RPATH also names the directory that rexlib was found in. Windows
has no RPATH: `_dll_directories` adds the `bin` of the package directory and
of the prefix the default layout implies to the DLL search path, and in any
other layout the DLL has to be on the search path already. `_paths` finds
rexlib's CMake package and headers from where the loaded library is, whatever
the layout.

Everything the binding installs is in the `rexlib_python` component. The
type stubs are generated last, by importing the installed package; see
[type-stubs](type-stubs.md).

CMake 3.18 is the minimum, and 3.26 with `REXLIB_PYTHON_STABLE_ABI`, the
first whose FindPython has `Development.SABIModule`. The binding is C++20,
unlike rexlib itself, which is C++14: nothing here has to build on the
compilers rexlib supports, only on the ones that build wheels. CMake
requires that standard rather than preferring it, so that nothing newer gets
in unnoticed. The binding's targets get the library's warnings, which
`REXLIB_WARNINGS_AS_ERRORS` turns into errors.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_BUILD_PYTHON` | `OFF` | Adds the binding to the library's build |
| `REXLIB_USE_SYSTEM_DLPACK` | `OFF`, forced `ON` by `REXLIB_USE_SYSTEM_DEPENDENCIES` | Finds an installed DLPack rather than building it from `third_party/` |
| `REXLIB_PYTHON_INSTALL_DIR` | The interpreter's platform library directory, relative to the prefix; under scikit-build-core, the wheel's | Where the package is installed |
| `REXLIB_PYTHON_STABLE_ABI` | `OFF`, `ON` from `pyproject.toml` | Builds the extension against the stable ABI of Python 3.12 |

Built alone, the binding also reads the library's `REXLIB_INSTALL`,
`REXLIB_BUILD_TESTING`, `REXLIB_WARNINGS_AS_ERRORS` and
`REXLIB_USE_SYSTEM_DEPENDENCIES`.

`REXLIB_PYTHON_STABLE_ABI` is what makes one wheel serve Python 3.12 and every
later one, with `wheel.py-api = "cp312"` beside it in `pyproject.toml`.
nanobind falls back to the interpreter's own ABI where the stable one cannot
serve — before 3.12, and on a free-threaded interpreter — and
scikit-build-core tags those wheels accordingly. The suffix of the module is
nanobind's to choose from what scikit-build-core reports, which is the target
interpreter's when cibuildwheel cross-compiles.

Its CMakeLists.txt files name their sources as the library's do (see
[layout](layout.md)) and add them to `_binding`. nanobind and DLPack are in
[dependencies](dependencies.md).

## The CUDA plugin

The plugin is built as any third-party plugin is: against an installed
rexlib, which it finds with `find_package`, of the minor version it shares
with the library. From the root of the repository, install the library into
a prefix and point `rexlib_ROOT` at it, on the command line or in the
environment:

```
cmake -S . -B build/rexlib -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build/rexlib --parallel
cmake --install build/rexlib --prefix build/rexlib-install
cmake -S plugins/cuda -B build/cuda -Drexlib_ROOT=build/rexlib-install
cmake --build build/cuda --parallel
```

With CMake 3.21 or newer the presets of `plugins/cuda/` do the same, run from
that directory: `cmake --preset debug`, `cmake --build --preset debug`, `ctest
--preset debug`, which build into `plugins/cuda/build/debug`. There are
`release`, `sanitize` (the address and undefined behaviour sanitizers),
`sanitize-thread` and `coverage` presets too; the two sanitizer presets are not
offered on Windows, and `coverage` only on Linux. The `sanitize` test preset
sets `ASAN_OPTIONS=protect_shadow_gap=0`, without which the address sanitizer
and the CUDA runtime fight over the same memory. The `sanitize-thread` one
points `TSAN_OPTIONS` at `tests/tsan.supp`, which leaves out the lock cycles
that go through trompeloeil's own lock: a mocked call holds it through its
actions, and a case that calls back into the code under test from one takes
that code's lock under it. What only one machine has, such as `rexlib_ROOT`,
the CUDA paths or a compiler cache, goes in a `CMakeUserPresets.json`, which
git ignores.

A CUDA toolkit of 11 or newer is required. Unless `CMAKE_CUDA_ARCHITECTURES`
or the environment's `CUDAARCHS` names them, the architectures are CMake's
`all-major`: the oldest of each major family the toolkit supports as SASS,
plus the PTX of the newest. CUDA is enabled with `enable_language` after that,
not in `project()`.

nvcc accepts only the host compilers its release supports, and the C library
headers of their time: a GCC or glibc newer than the toolkit fails as soon as
CUDA is enabled, before any source is compiled. `CUDAHOSTCXX` names another
host compiler, as the wheel builds do; a glibc too new needs a newer toolkit.

CMake 3.24 is the minimum, the first with `all-major`. The plugin is C++20,
and CMake requires that standard rather than preferring it, so that nothing
newer gets in unnoticed.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_CUDA_BUILD_TESTING` | `ON` while `BUILD_TESTING` is on, forced `OFF` otherwise | Builds the test suites |
| `REXLIB_CUDA_REGISTER_TESTS_PER_BINARY` | `OFF` | Registers one CTest entry per test executable instead of one per Catch2 case |
| `REXLIB_CUDA_INSTALL` | `ON` | Generates the install rules |
| `REXLIB_CUDA_WARNINGS_AS_ERRORS` | `OFF` | Makes a compiler warning fail the plugin's own targets (`-Werror`, `/WX`) |

The sources are compiled once, into the object library `rexlib-cuda-objects`,
which both the plugin and the unitary tests are built from: a `MODULE` cannot
be linked against. Their usage requirements travel through the interface
target `rexlib-cuda-deps`, because `$<TARGET_OBJECTS:>` carries only the
objects. Device code is compiled separately and linked into the module, so a
kernel may be called across translation units.

Its CMakeLists.txt files name their sources as the library's do (see
[layout](layout.md)).

The plugin installs through rexlib's `rexlib_install_plugin`, which owns where
a plugin goes and the RPATH back to the library.

## Coverage

`REXLIB_ENABLE_COVERAGE` adds `-pthread` beside `--coverage` under GCC. The
counters gcov keeps are one per arc and shared by every thread running the
code, and the CPU backend runs its loops on a thread pool, so updated the
ordinary way the increments race and are lost. That does not merely
undercount: gcov measures a subset of the arcs and derives the rest by
conservation of flow, so counters that disagree make a derived branch come out
negative, and gcovr stops at the first one. The GCC driver selects
`-fprofile-update=prefer-atomic` when `-pthread` is on the compile line, which
is the whole reason the flag is named there; nothing else sets it, since
`THREADS_PREFER_PTHREAD_FLAG` is unset and `Threads::Threads` only links.

Naming `-fprofile-update=atomic` directly would say the same thing and is what
the option is for, but ccache learned that option in 4.10 and treats it as
unsupported before then, making every instrumented unit uncacheable. The
runners carry 4.5.1 on jammy, 4.9.1 on noble and on Windows, so all but macOS
would recompile the whole library on every run. `-pthread` is an ordinary
flag every version caches.

Clang ignores `-pthread` here — its driver acts on `-fprofile-update` alone —
so its counters are not atomic and its counts are lost the same way. It is
left as it is because clang selects arcs differently and has never produced a
negative, and because buying atomic counters there costs the cache.
