# Working on rexlib-cuda

rexlib-cuda is the CUDA hardware backend of
[rexlib](https://github.com/gigabit-clowns/rexlib). It is a plugin: a shared
object that rexlib's plugin manager loads at run time, which registers a CUDA
`device_backend` with the device manager. It has no public headers and no
interface of its own beyond the one rexlib defines.

Keep this file true. When a change makes something here wrong or missing —
a moved directory, a new convention, a dependency, a workflow — update it in
the same pull request that causes it.

## Layout

| Path | Holds |
|---|---|
| `src/plugin.hpp`, `src/plugin.cpp` | `cuda_plugin`, the `plugin` the core sees: its name, version and what it registers |
| `src/plugin_hook.cpp` | `rexlib_get_plugin`, the one exported symbol |
| `src/hardware/` | The implementation of rexlib's hardware interface: backend, device, queue, event, memory resources, errors |
| `src/hardware/caching_allocator/` | The caching memory allocator, modelled on PyTorch's |
| `src/config.hpp` | Every tunable of the backend, as a macro the build may override |
| `src/logger.hpp` | The `REXLIB_CUDA_LOG_*` macros over spdlog |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `cmake/modules/` | `fetch_boost`, `fetch_spdlog` and `cuda_architectures` |
| `scripts/select_cuda_major.py` | Rewrites `pyproject.toml` for one CUDA major before a wheel is built |
| `external/rexlib` | The rexlib this plugin is developed against, a submodule |

Everything is in `namespace rexlib::cuda`, except `cuda_plugin`, which is in
`namespace rexlib`.

## Building

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

A CUDA toolkit of 11 or newer is required. Unless `CMAKE_CUDA_ARCHITECTURES`
is set, `cuda_all_major_architectures` asks nvcc which architectures it
supports and keeps the oldest of each major family as SASS, plus the PTX of
the newest. That needs CUDA 11.1; an older toolkit needs the architectures set
by hand. CUDA is enabled with `enable_language` after that, not in
`project()`.

nvcc accepts only the host compilers its release supports, and the C library
headers of their time: a GCC or glibc newer than the toolkit fails as soon as
CUDA is enabled, before any source is compiled. `CUDAHOSTCXX` names another
host compiler, as the wheel builds do; a glibc too new needs a newer toolkit.

CMake 3.18 is the minimum. The plugin is C++20, with
`CMAKE_CXX_STANDARD_REQUIRED` off.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_CUDA_BUILD_TESTING` | `BUILD_TESTING` | Builds the test suites |
| `REXLIB_CUDA_INSTALL` | `ON` | Generates the install rules |
| `REXLIB_CUDA_REXLIB_VERSION` | `0.1` | The rexlib version `find_package` asks for |
| `REXLIB_CUDA_RUNTIME_RPATH` | empty | An extra RPATH entry reaching the CUDA runtime |

The sources are compiled once, into the object library `rexlib-cuda-objects`,
which both the plugin and the unitary tests are built from: a `MODULE` cannot
be linked against. Their usage requirements travel through the interface
target `rexlib-cuda-deps`, because `$<TARGET_OBJECTS:>` carries only the
objects. Device code is compiled separately and linked into the module, so a
kernel may be called across translation units. `src/CMakeLists.txt` globs its
sources.

The plugin installs through rexlib's `rexlib_install_plugin`, which owns where
a plugin goes and the RPATH back to the library.

## Dependencies

Boost (container, intrusive, unordered) and spdlog are fetched and built as
static libraries, through `cmake/modules/fetch_*.cmake`; their versions are
literals in `CMakeLists.txt`. Catch2 and trompeloeil, for the tests, are
fetched by URL in `tests/CMakeLists.txt`. Renovate follows all four through
custom managers in `renovate.json`.

The `cuda-toolkit` requirement in `pyproject.toml` is not a version this
project follows: it is a placeholder `select_cuda_major.py` rewrites for each
wheel, and Renovate leaves it alone.

## Conventions

Indentation is tabs. Lines stay within 80 columns.

Declarations go in a `.hpp`, definitions that must be visible go in an `.inl`
the header includes at its end, and everything else goes in a `.cpp`. Every
source starts with `// SPDX-License-Identifier: GPL-3.0-only`.

Declarations are documented with Doxygen comments: `@brief`, `@param`,
`@return` and `@throws`.

Every call into the CUDA runtime goes through one of three macros from
`src/hardware/error.hpp`, which record the call and where it was made:

| Macro | On failure |
|---|---|
| `REXLIB_CUDA_CHECK` | Throws `cuda::error` |
| `REXLIB_CUDA_CHECK_ALLOCATION` | Throws `std::bad_alloc` when memory ran out, `cuda::error` otherwise |
| `REXLIB_CUDA_CHECK_NO_THROW` | Writes to the standard error stream; for destructors and other `noexcept` code |

Logging goes through `REXLIB_CUDA_LOG_TRACE` … `REXLIB_CUDA_LOG_CRITICAL`, and
`REXLIB_CUDA_LOG_LEVEL` sets the level compiled in.

The plugin and the core it is loaded into have to agree on compiler, standard
library and, on Windows, the runtime: the plugin boundary is a C++ ABI
boundary. See rexlib's AGENTS.md.

## Tests

```
ctest --test-dir build/cuda --output-on-failure
```

The unitary suite embeds the plugin's objects and exercises its internals,
mostly the caching allocator against mocks. The integration suite loads the
built plugin through rexlib's plugin manager, which is why the plugin is
copied beside the test executable. A test that needs a GPU skips itself when
no CUDA capable device is present, which is always the case in CI.

On Windows the rexlib DLL is copied beside each test executable, since nothing
else puts it on the loader's search path.

## Wheels

One repository builds one distribution per CUDA major: `rexlib-cuda11`,
`rexlib-cuda12` and `rexlib-cuda13`. `select_cuda_major.py` rewrites the
distribution name and the `cuda-toolkit` requirement before each build, line by
line so that the comments in `pyproject.toml` survive.

A wheel is Python-agnostic (`wheel.py-api = "py3"`) and manylinux only. It
installs into the `rexlib` package directory (`wheel.install-dir = "rexlib"`),
because rexlib reports its plugin directory relative to its own prefix. The
plugin's RPATH reaches the CUDA runtime of NVIDIA's own wheels, three levels
up in site-packages, through `REXLIB_CUDA_RUNTIME_RPATH`, and auditwheel is
told to leave both librexlib and libcudart out of the wheel.

The wheels build against a rexlib built from `external/rexlib` beforehand, in
the same container, not against an installed `rexlib` wheel.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

| Workflow | Does |
|---|---|
| `build-and-test.yml` | Builds and tests with CMake, then the SonarQube scan |
| `deploy.yml` | Builds the wheels for each CUDA major on x86_64 and aarch64, and publishes them |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml`, `keep-caches-warm.yml` | Manage the Actions cache |

`build-and-test.yml` covers Ubuntu with gcc and clang on CUDA 12, Ubuntu 22.04
with CUDA 11 and Windows with CUDA 13, in Debug. It builds rexlib from the
submodule into a prefix first, which is most of the wall clock and what the
compiler cache is for. The cache and the fetched sources are saved only on
`main`. The compilation database of the gcc build is what the scan reads.

Publishing to PyPI is wired up but disabled. Every push to `main` updates the
`development` pre-release with its wheels.
