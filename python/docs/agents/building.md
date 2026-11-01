# Building

How to build and install the package, the CMake options, and why the build is
set up as it is.

The binding needs a rexlib to link. The submodule is the one it is developed
against, so clone with it:

```
git clone --recurse-submodules https://github.com/gigabit-clowns/rexlib-python.git
```

Build that submodule into a prefix, then install this project against it:

```
cmake -S external/rexlib -B build/rexlib -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=~/.local -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF
cmake --build build/rexlib --target install --parallel $(nproc)

CMAKE_PREFIX_PATH=~/.local CMAKE_BUILD_PARALLEL_LEVEL=$(nproc) \
  pip install . --no-build-isolation -v -Ccmake.define.REXLIB_PYTHON_BUILD_TESTING=ON
```

`pip install` is the way in, not `cmake --build`: scikit-build-core drives
CMake, and the install rules are what put the package together. A CMake build
on its own is what CI uses to get a compilation database for the scanner, not
a way to get a working package.

Installed this way the package carries no rexlib, and the extension's RPATH
names, after the package's own library directory, the one rexlib was found
in, so the package works where it was built. Windows has no RPATH: there the
rexlib DLL has to be on the loader's search path. A wheel carries rexlib, as
does any build given `REXLIB_PYTHON_STAGE_PREFIX` or one that builds
`external/rexlib`, and names nothing outside itself.

That build is thrown away with its directory, so installing twice compiles the
binding twice. `SKBUILD_BUILD_DIR` names one to keep and is left to whoever is
installing rather than set here — the README carries it. It is deliberately
not in `pyproject.toml`: the gain is a developer's repeated install, while the
cost, a CMake cache that outlives the options it was configured with, would
fall on every user and on six wheel jobs that already hit their compiler cache
without it.

CMake 3.18 is the minimum, and 3.26 with `REXLIB_PYTHON_STABLE_ABI`, the
first whose FindPython has `Development.SABIModule`. The binding is C++20,
unlike rexlib itself, which is C++14: nothing here has to build on the
compilers rexlib supports, only on the ones that build wheels.
`CMAKE_CXX_STANDARD_REQUIRED` is off all the same.

nanobind and DLPack are fetched, each pinned in `CMakeLists.txt` as a git tag
that Renovate follows. DLPack is one header, and only the header is taken:
its own build file is not read. rexlib is found with `find_package` and only
built from `external/rexlib` when none is found, which is what makes a source
distribution installable on a machine with no rexlib.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_PYTHON_BUILD_TESTING` | `BUILD_TESTING` | Builds the dummy plugins the tests need |
| `REXLIB_PYTHON_INSTALL` | `ON` | Generates the install rules |
| `REXLIB_PYTHON_INSTALL_DIR` | `Python_SITEARCH`; under scikit-build-core, always the wheel's platlib | Where the package is installed |
| `REXLIB_PYTHON_STAGE_PREFIX` | empty | A rexlib prefix to copy inside the package |
| `REXLIB_PYTHON_STUBS_DIR` | empty | Pre-generated `.pyi` files to install instead of generating them |
| `REXLIB_PYTHON_STABLE_ABI` | `OFF`, `ON` from `pyproject.toml` | Builds the extension against the stable ABI of Python 3.12 |
| `REXLIB_PYTHON_REXLIB_VERSION` | `0.1` | The rexlib version `find_package` asks for |

`REXLIB_PYTHON_BUILD_TESTING` is separate from `BUILD_TESTING` because
dependencies of dependencies write that one into the cache, and this project's
tests should not be at their mercy.

`REXLIB_PYTHON_STABLE_ABI` is what makes one wheel serve Python 3.12 and every
later one, with `wheel.py-api = "cp312"` beside it in `pyproject.toml`.
nanobind falls back to the interpreter's own ABI where the stable one cannot
serve — before 3.12, and on a free-threaded interpreter — and
scikit-build-core tags those wheels accordingly. The suffix of the module is
nanobind's to choose from what scikit-build-core reports, which is the target
interpreter's when cibuildwheel cross-compiles.

`REXLIB_PYTHON_STAGE_PREFIX` is copied whole, so it has to hold rexlib and
nothing else, and no symlinks — a wheel stores a symlink as a second full copy
of a 50 MB library. rexlib's `rexlib_runtime` and `rexlib_development`
components install exactly that.

Unlike rexlib, `src/CMakeLists.txt` globs its sources. A new `.cpp` is picked
up by reconfiguring and needs naming nowhere. The reasons rexlib lists its
sources explicitly do not apply here: there is one target, and nothing in the
binding registers itself through objects at namespace scope, so link order
decides nothing.
