# Building

How the binding is built, inside the library's build or alone, and its
options.

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
prefix is the package directory itself. Its `cmake.root` entry point names
the package, so that a project built with scikit-build-core against the
installed wheel finds rexlib's CMake package with `find_package(rexlib)`. The build writes the path from the
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

nanobind comes from pip and is never fetched: it is in `build-system.requires`
for pip, and in the `dev` group for a build by CMake itself. CMake asks the
interpreter it found where nanobind's package is, `python -m nanobind
--cmake_dir`, unless `nanobind_ROOT` or `nanobind_DIR` says so.

DLPack is fetched by default, through `rexlib_python_fetch_dlpack()` in
`cmake/modules/`, pinned as `COMMIT <sha>) # <tag>` in `CMakeLists.txt`,
which Renovate keeps up to date. It is one header, and only the header is
taken: its own build file is not read. The library's
`REXLIB_USE_SYSTEM_DEPENDENCIES`, or `REXLIB_USE_SYSTEM_DLPACK` alone, on
finds an installed one instead; no version is asked of it, since its package
reports 0.6 whatever the release.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_BUILD_PYTHON` | `OFF` | Adds the binding to the library's build |
| `REXLIB_USE_SYSTEM_DLPACK` | `OFF`, forced `ON` by `REXLIB_USE_SYSTEM_DEPENDENCIES` | Finds an installed DLPack rather than fetching it |
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

As in rexlib, every directory holding sources carries a `CMakeLists.txt`
naming them, which adds them to `_binding` and descends into the directories
below it. A new `.cpp` has to be named there before it is built: the lists
are explicit rather than globbed, so that adding a file cannot go unnoticed
by the build system.
