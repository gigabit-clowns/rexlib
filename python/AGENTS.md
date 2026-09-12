# Working on rexlib-python

rexlib-python is the Python binding for [rexlib](https://github.com/gigabit-clowns/rexlib),
a device-agnostic array computing library with a pluggable backend
architecture. It is a binding and nothing more: every computation happens in
C++, and this repository decides only what the Python side of that API looks
like.

Keep this file true. When a change makes something here wrong or missing —
a moved directory, a new convention, a dependency, a workflow — update it in
the same pull request that causes it.

## Layout

| Path | Holds |
|---|---|
| `src/` | The pybind11 binding, 35 `.cpp` and 35 `.hpp`, compiled into the `rexlib._binding` extension |
| `python/rexlib/` | The Python package, 8 `.py`: everything the binding cannot express |
| `tests/` | pytest suites, mirroring the binding's module structure |
| `tests/assets/` | Two dummy plugins, built by CMake, that the plugin tests discover |
| `scripts/` | Development tools: the test runner, the stub generator, the wheel check |
| `cmake/modules/` | `python_stubs.cmake`, which installs the type stubs |
| `conf/` | `coverage.ini` |
| `external/rexlib` | The rexlib the repository is developed against, a submodule |

There are two sides, and which one a change belongs to is the first question
to ask.

`src/` mirrors rexlib's own structure one directory per namespace, and the
module it produces mirrors the C++ API: same names, same argument order, same
required arguments. It does not decide anything. `python/rexlib/` is where the
API becomes Python: default arguments, implicit context, operators,
process-wide state. Everything in it is private — a leading underscore on
every module — and `__init__.py` is the whole public surface.

The rule that follows: if a change could be expressed in either place, it goes
in `python/rexlib/`. C++ is the expensive side to change and the one that has
to keep matching rexlib.

### The binding's modules

`_binding` carries at its top level what rexlib declares in its own namespace
root, and one submodule per rexlib namespace:

| Module | From | Holds |
|---|---|---|
| `_binding` | `src/core/*.cpp` | `Version`, `Plugin`, `PluginManager`, `ServiceCatalog`, `rexlib_version`, `rexlib_binding_version` |
| `_binding.numerical` | `src/core/numerical/` | `NumericalType`, and the `float16_t` type caster |
| `_binding.ndarray` | `src/core/ndarray/` | `Array`, `ArrayDescriptor` |
| `_binding.hardware` | `src/core/hardware/` | Devices, sessions, queues, events, memory resources |
| `_binding.dispatch` | `src/core/dispatch/` | `ExecutionContext`, `Dispatcher`, `ProgramManager` |
| `_binding.functional` | `src/functional/` | The operations, each taking an explicit context |

`src/main.cpp` is the only place that creates a submodule, and it names them
in the order the declarations need.

### The package's modules

| Module | Adds |
|---|---|
| `_paths.py` | Locates the rexlib shipped inside the package; on Windows puts it on the DLL search path |
| `_catalog.py` | The process-wide default `ServiceCatalog` |
| `_session_pool.py` | One `DeviceSession` per device, process-wide |
| `_context.py` | The execution context active on the current thread |
| `_device.py` | `rexlib.device(...)`, the `with` block that activates one |
| `_functional.py` | The operations again, with `context` defaulting to the active one |
| `_ndarray.py` | Installs the Python operators onto `Array` |

`_paths` is imported first in `__init__.py`, and the order matters: on Windows
nothing else imports until the bundled library is findable. `_ndarray` is
imported for its side effect, before anything can hand out an `Array`.

## Building

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

CMake 3.18 is the minimum. The binding is C++20, unlike rexlib itself, which
is C++14: nothing here has to build on the compilers rexlib supports, only on
the ones that build wheels. `CMAKE_CXX_STANDARD_REQUIRED` is off all the same.

pybind11 is fetched, pinned in `CMakeLists.txt` as a git tag. rexlib is found
with `find_package` and only built from `external/rexlib` when none is found,
which is what makes a source distribution installable on a machine with no
rexlib.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_PYTHON_BUILD_TESTING` | `BUILD_TESTING` | Builds the dummy plugins the tests need |
| `REXLIB_PYTHON_INSTALL` | `ON` | Generates the install rules |
| `REXLIB_PYTHON_INSTALL_DIR` | The wheel's platlib, else `Python_SITEARCH` | Where the package is installed |
| `REXLIB_PYTHON_STAGE_PREFIX` | empty | A rexlib prefix to copy inside the package |
| `REXLIB_PYTHON_STUBS_DIR` | empty | Pre-generated `.pyi` files to install instead of generating them |
| `REXLIB_PYTHON_MODULE_SUFFIX` | `$SETUPTOOLS_EXT_SUFFIX` | The extension suffix, for cross-compiled wheels |
| `REXLIB_PYTHON_REXLIB_VERSION` | `0.1` | The rexlib version `find_package` asks for |

`REXLIB_PYTHON_BUILD_TESTING` is separate from `BUILD_TESTING` because
dependencies of dependencies write that one into the cache, and this project's
tests should not be at their mercy.

`REXLIB_PYTHON_STAGE_PREFIX` is copied whole, so it has to hold rexlib and
nothing else, and no symlinks — a wheel stores a symlink as a second full copy
of a 50 MB library. rexlib's `rexlib_runtime` and `rexlib_development`
components install exactly that.

Unlike rexlib, `src/CMakeLists.txt` globs its sources. A new `.cpp` is picked
up by reconfiguring and needs naming nowhere. The reasons rexlib lists its
sources explicitly do not apply here: there is one target, and nothing in the
binding registers itself through objects at namespace scope, so link order
decides nothing.

## Type stubs

The extension's `.pyi` files are generated while it is built, by
`scripts/generate_stubs.py` through pybind11-stubgen, and land in
`rexlib/_binding/` beside it. They are not in the repository and there is
nothing to run by hand.

Generating them imports the extension, which a cross-compiling build cannot
do. Those builds are handed stubs made elsewhere through
`REXLIB_PYTHON_STUBS_DIR`; stubs describe the Python API, so the same ones are
correct on every platform. A build that can neither generate nor be given them
fails rather than producing an untyped package.

The package is typed — `py.typed` ships — so a change to the binding's
signatures is a change to the stubs, generated for free, and a change to
`python/rexlib/` is only as typed as it was written.

## Conventions

### The binding

Every bound entity gets a `.hpp` and a `.cpp`. The header declares a class
alias and the two functions the source defines:

```cpp
using device_class = pybind11::class_<device, PyDevice, std::shared_ptr<device>>;

device_class declare_device(pybind11::module_ &m);
void define_device(device_class &c);
```

`declare_X` creates the type and nothing else. `define_X` fills it in. A
namespace's `main.cpp` calls every `declare_` first and every `define_`
afterwards, which is what lets any binding refer to any other type regardless
of the order they appear in — a default argument, a return type, a base class.
Splitting the two is the whole point; do not collapse them because a
particular type happens not to need it today.

`define_X` takes a second `pybind11::module_ &` when it also registers
something at module level, which is how the free functions beside a type are
bound: `define_device_manager` adds `get_device_manager`, `define_version`
adds the `rexlib_version` attribute.

Managers are reached through a module-level getter taking a `ServiceCatalog`,
never constructed: `get_device_manager(catalog)`, `get_program_manager(catalog)`.
They are services the catalog owns.

Names follow rexlib on the C++ side — `lower_case`, `m_` on private members —
and Python on the bound side: `CamelCase` types, `lower_case` functions and
properties. A C++ getter pair becomes a property, not `get_x()`/`set_x()`.

Every argument is named with `py::arg`. An optional out parameter is
`std::optional<array*>` with a `py::none()` default, unwrapped by a static
`py_` function beside the binding; that function exists because the C++
signature takes a raw pointer and pybind11 will not produce one from `None`.

Lifetimes are stated: `py::keep_alive` where an object borrows from another,
`py::return_value_policy::reference_internal` where a getter hands out a
reference into its owner. Getting this wrong does not fail a test, it crashes
a user.

A rexlib type with no Python counterpart gets a `type_caster`, not a bound
class — see `float16_caster.hpp`, which converts `float16_t` through `float`
and thereby makes `std::complex<float16_t>` work too.

`PyX` trampolines live in the header, not the source, because they are part of
the `class_` alias callers see.

The binding builds with `-Wall -Wextra -Wpedantic`, `/W4 /WX` on MSVC.

### The package

Indentation is tabs, on both sides. Lines stay within 80 columns.

The package targets Python 3.9, so `from __future__ import annotations` goes
at the top of every module that annotates anything, and `X | None` is written
only under it.

Module-level state uses two leading underscores — `__pool`, `__default_catalog`,
`__local`. Python mangles nothing at module scope, so this is a convention
rather than a mechanism, and it is the one that marks the difference between
the module's own state and something a caller might reasonably reach for.

Public functions carry a docstring with `Args:` and `Returns:`; ruff enforces
their presence under pydocstyle, in pep257 convention.

`__init__.py` re-exports with `as`, `from ._x import y as y`, which is what
makes the name public to a type checker, and lists everything again in
`__all__`. Both are needed and both are checked.

Operations live at the top of `rexlib`, not in a submodule, matching the C++
side where they are in `rexlib` itself and the array libraries they are used
alongside. `dispatch` and `hardware` stay submodules: they are reached for
when setting up, not when computing.

ruff is the authority and runs in CI as `ruff check .`. Its configuration is
`ruff.toml`; `D206` is disabled because the project indents with tabs.

### Comments

Say **what**, never **why**. The why belongs in the commit message and the
pull request. And only say the what when the code does not already: if a
reader understands the line without the comment, the comment does not go in.

There is not a single `TODO` in the repository. Unfinished work is an issue,
not a comment.

## Tests

They are Python tests against the installed package. There are no C++ tests:
the binding has no logic of its own worth testing from C++, and what it does
have is only observable from Python.

```
./scripts/run-tests.sh
./scripts/run-tests.sh --coverage
```

The package has to be installed with `REXLIB_PYTHON_BUILD_TESTING=ON`, which
is what builds the dummy plugins in `tests/assets/`. The runner sets pytest's
rootdir to the repository and, with `--coverage`, writes `coverage.xml`
against `conf/coverage.ini`, which maps `python/rexlib` onto the installed
copy the tests actually import.

`tests/` mirrors the binding's module structure. A test goes in the directory
named after the module whose surface it exercises, not the module it happens
to call through: `tests/dispatch/test_active_context.py` reaches `rexlib.zeros`
to observe context resolution, and context resolution is what it is about.
Tests for what `_binding` exposes at its top level stay at the root.

| Directory | Covers |
|---|---|
| `tests/` | `Version`, `PluginManager`, `ServiceCatalog` |
| `tests/numerical/` | `NumericalType` |
| `tests/ndarray/` | `ArrayDescriptor`, and the operators installed onto `Array` |
| `tests/hardware/` | Devices, sessions, events, memory resources, the session pool |
| `tests/dispatch/` | `ExecutionContext`, the active context, `rexlib.device(...)` |
| `tests/functional/` | The operations |

There are no `__init__.py` files and no `conftest.py`. Test module names are
therefore unique across the whole tree, and a fixture belongs to the file that
uses it — several files define a `__setup_context` of their own rather than
sharing one. Keep new fixtures file-local until something actually needs to
share them.

`tests/CMakeLists.txt` descends into `assets/` and nothing else: the Python
files are collected by pytest from where they are, so moving one needs no
change to CMake.

## Continuous integration

| Workflow | Does |
|---|---|
| `build-and-test.yml` | Builds with CMake and with pip across the matrix, runs ruff and the suites, then the SonarQube scan |
| `deploy.yml` | Generates the stubs, builds the wheels and the source distribution, and publishes them |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml`, `keep-caches-warm.yml` | Manage the Actions cache |

`build_with_cmake` covers Linux with gcc and clang, macOS, and Windows on both
architectures; it exists to keep the plain CMake path working and to produce
the compilation database the scanner reads. `build_with_pip` is the one that
runs the tests, across five platforms and Python 3.9 through 3.14, minus the
Windows ARM entries for the versions CPython never shipped there.

Both build rexlib first, into a prefix outside the project, and point the
build at it with `rexlib_ROOT`. It is the bulk of the wall clock, and ccache
is what makes every run after the first a link. The compiler cache and the
fetched sources are saved only on `main`: a pull request reads them and writes
nothing, so one branch cannot evict another's.

Every job moves the submodule to rexlib's `main` before building. That step is
temporary and goes away, along with the Renovate rule holding the submodule
back, once rexlib tags releases.

Coverage is produced by one entry only, Linux on 3.14, and handed to the scan
as an artifact.

### Wheels

`deploy.yml` generates the stubs once, in a job of its own, and hands them to
every wheel — a cross-compiled wheel cannot make its own. The wheels then
build rexlib from `external/rexlib` rather than being pointed at one, so that
cibuildwheel's architecture, deployment target and SDK apply to it too instead
of having to be repeated to a step outside.

The wheel carries librexlib itself, put there by this project's install rules,
and every repair tool is told to leave it alone: `auditwheel --exclude`,
`delvewheel --exclude`, `delocate --ignore-missing-dependencies`. Vendoring it
would rename it under a hash-mangled SONAME, which breaks `find_package` for
plugin authors and the sibling directory the plugins are discovered in. The
namelink is excluded from the wheel instead, since a wheel stores a symlink as
another full copy.

musllinux is skipped: NVIDIA ships no CUDA for musl, so rexlib-cuda is
manylinux only and an Alpine install could never reach the GPU backend. Alpine
can still build from the source distribution.

`scripts/check_wheels_agree.py` asserts that every wheel of a platform carries
the same rexlib, which is what makes reusing rexlib's objects between
interpreters sound rather than assumed.

The source distribution names `external/rexlib/**` explicitly, because
`git ls-files` does not report a submodule's contents and the archive would
otherwise carry an empty directory and fail to build.

Publishing to PyPI is wired up but disabled. Every push to `main` updates the
`development` pre-release with its binaries.
