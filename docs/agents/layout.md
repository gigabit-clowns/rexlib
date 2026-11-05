# Layout

Where the library, its Python binding and its CUDA plugin live, with their
tests and build files.

| Path | Holds |
|---|---|
| `include/rexlib/` | The public headers of the library |
| `src/` | The library's implementation, plus headers that are not public |
| `src/backends/cpu/` | The CPU backend, built into the library: builders, kernels, loops, plans |
| `src/core/` | What an operation is and which operands it admits (`operation/`, with the shape and type rule languages in `shape_rule/` and `type_rule/`), how it runs (`dispatch/`), layouts, hardware abstraction, plugin loading |
| `src/ops/`, `src/functional/` | Operation declarations and the functions that reach them |
| `src/em/` | The Cryo-EM and Cryo-ET operations and the functions that reach them, in namespace `rexlib::em`. Their CPU programs live with the others, under `src/backends/cpu/builders/em/` |
| `tests/unitary/`, `tests/integration/` | The library's Catch2 suites, with trompeloeil for mocks |
| `tests/headers/` | The check that compiles each public header on its own, under every C++ standard the compiler has |
| `tests/install/` | The test that installs rexlib and builds `plugin/` and `consumer/`, projects of their own, against the install |
| `tests/python/` | The binding's pytest suites, mirroring its module structure, with two dummy plugins in `assets/` |
| `benchmarks/` | Catch2 benchmarks, mirroring the layout of `src/`, which reuse the fixtures of `tests/unitary/` |
| `python/src/` | The nanobind binding, compiled into the `rexlib._binding` extension |
| `python/rexlib/` | The Python package: everything the binding cannot express |
| `python/DECISIONS.md`, `python/BACKLOG.md` | The binding's open questions, and its settled work waiting to be scheduled |
| `plugins/cuda/` | The CUDA plugin, a CMake project of its own built against an installed rexlib, or inside the library's build with `REXLIB_BUILD_CUDA_PLUGIN`: `src/`, `tests/`, its own `cmake/modules/` and presets |
| `plugins/cuda/cuda12/`, `plugins/cuda/cuda13/` | The `pyproject.toml` of the plugin's wheel for each CUDA major, built from the project one directory up |
| `cmake/modules/` | One `rexlib_fetch_*.cmake` per dependency of the library and `rexlib_python_fetch_dlpack.cmake` for the binding, plus the `Find*.cmake` for those that ship no package config |
| `cmake/config/` | The templates of the installed CMake package and of the version header, and `rexlib-plugin.cmake`, the CMake helpers of the plugin SDK |
| `docs/doxygen/` | The reference of the C++ headers, with Doxygen |
| `docs/templates/source/` | Templates of a header, an inline file and a source |
| `docs/agents/` | The docs that `AGENTS.md` indexes |
| `VERSION` | The version of the library, the binding and every distribution of a release; `plugins/cuda/VERSION` holds the plugin's, the same before 1.0 |
| `pyproject.toml`, `uv.lock` | The `rexlib` wheel, the dependency groups of development, and the lock that pins them |
| `vcpkg.json` | Every dependency of the library, the binding and the CUDA plugin, for vcpkg to install at the baseline it names; see [dependencies](dependencies.md) |
| `.github/workflows/` | `ci.yml`, which every pull request runs, and `release.yml`; see [ci](ci.md) |

## The library

The same top level groups appear on both sides, `core`, `backends`, `ops`,
`functional` and `em`, but not every directory has a counterpart: a header with
no implementation of its own lives only under `include/`.

Image I/O is not part of rexlib. It lives in
[vitrio](https://github.com/gigabit-clowns/vitrio), a library of its own.

Every directory holding sources carries a `CMakeLists.txt` naming them, which
adds them to the component target its group belongs to and descends into the
directories below it. A new `.cpp` has to be named there before it is built:
the lists are explicit rather than globbed, so that adding a file cannot go
unnoticed by the build system and so that link order does not follow whatever
order the file system returns. That order matters here, since the CPU program
builders register themselves through objects at namespace scope. The binding
and the plugin follow the same rule.

## The binding

There are two sides, and which one a change belongs to is the first question
to ask.

`python/src/` mirrors rexlib's own directory structure, and the module it
produces mirrors the C++ API: same names, same argument order, same required
arguments. It does not decide anything. `python/rexlib/` is where the API
becomes Python: default arguments, implicit context, operators, process-wide
state. Its modules carry a leading underscore and `__init__.py` re-exports
what they hold, so that the public surface is one flat namespace rather than a
set of import paths.

A public subpackage is the exception to that, and `em/` is the one: its module
path *is* the API, so the directories are named without an underscore and each
carries an `__all__` of its own. Anything private inside one still takes the
underscore.

`em` holds the operations of `rexlib::em`, which rexlib declares under
`em/functional/` the way it declares its own under `functional/`. They are
flat in `rexlib.em`, as the operations of `rexlib` are flat in `rexlib`, each
with its context defaulted by `em/_functional.py`.

The rule that follows: if a change could be expressed in either place, it goes
in `python/rexlib/`. C++ is the expensive side to change and the one that has
to keep matching rexlib.

## The CUDA plugin

| Path under `plugins/cuda/` | Holds |
|---|---|
| `src/cuda_plugin.hpp`, `src/cuda_plugin.cpp` | `cuda_plugin`, the `plugin` the core sees: its name, version and what it registers |
| `src/plugin_hook.cpp` | `REXLIB_DEFINE_PLUGIN`, which exports the plugin's two entry points |
| `src/hardware/` | The implementation of rexlib's hardware interface: backend, device, queue, event, memory resources, errors |
| `src/hardware/caching_allocator/` | The caching memory allocator, modelled on PyTorch's |
| `src/config.hpp` | Every tunable of the backend, as a macro the build may override |
| `src/logger.hpp` | The `REXLIB_CUDA_LOG_*` macros over spdlog |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `cmake/modules/` | One `rexlib_cuda_fetch_*.cmake` per dependency, `rexlib_cuda_architectures` and `rexlib_cuda_enable_warnings` |

Everything is in `namespace rexlib::cuda`.
