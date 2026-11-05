# Working on rexlib

rexlib is a high-performance Cryo-EM and Cryo-ET image processing library. It
is a C++ library with a Python binding, in `python/`, and its hardware
backends are plugins loaded at run time: the CPU backend is built into the
library, the CUDA plugin is in `plugins/cuda/`, and a third party writes its
own against the installed library.

This file is a map; the detail lives in `docs/agents/`. Keep both true. When a
change makes something in them wrong or missing — a moved directory, a new
convention, a dependency, a workflow — update them in the same pull request
that causes it.

## Building

The library and its tests, once the dependencies' submodules are checked out
(`git submodule update --init`), with the presets, which build the
dependencies from them:

```
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

With the binding, once the `dev` group of `pyproject.toml` is installed
(`uv sync`, or `pip install --group dev`): add
`-DREXLIB_BUILD_PYTHON=ON -DPython_EXECUTABLE=$(which python)` and run
`ctest --test-dir build/debug -L python`. The CUDA plugin builds against an
installed rexlib, as a third party's does, or, for development, inside the
library's build with `REXLIB_BUILD_CUDA_PLUGIN`; see
[building](docs/agents/building.md#the-cuda-plugin).

## Always

- C++14 in the library's sources and public headers; anything newer goes
  through the `REXLIB_*` feature macros. The binding and the CUDA plugin are
  C++20. [building](docs/agents/building.md)
- Every source file, the CMake files included, starts with
  `SPDX-License-Identifier: GPL-3.0-only`. [code style](docs/agents/code-style.md)
- Tabs, 80 columns, naming as `.clang-tidy` says. [code style](docs/agents/code-style.md)
- Declarations in a `.hpp`, visible definitions in an `.inl`, the rest in a
  `.cpp`. [code style](docs/agents/code-style.md)
- Comments say what, never why, and there is no `TODO`. [code style](docs/agents/code-style.md)
- A new `.cpp` is named in the `CMakeLists.txt` of its directory.
  [layout](docs/agents/layout.md)
- Test doubles are trompeloeil mocks. [test doubles](docs/agents/test-doubles.md)
- The binding mirrors the C++ API and decides nothing; a change that could go
  on either side goes in `python/rexlib/`. [layout](docs/agents/layout.md#the-binding)
- Every bound entity gets a `.hpp` and a `.cpp`, with a `declare_` and a
  `define_` function. [python binding](docs/agents/python-binding.md)
- A change to the binding is tested from Python, in the directory of the
  module whose surface it exercises. [tests](docs/agents/tests.md#the-binding)
- `ruff check .` passes. [python package](docs/agents/python-package.md)
- Every call into the CUDA runtime goes through a `REXLIB_CUDA_CHECK*` macro.
  [CUDA conventions](docs/agents/cuda-conventions.md)
- A plugin's static initializers do not call into librexlib.
  [plugins](docs/agents/plugins.md)
- A pull request is assigned to whoever opens it and labelled.
  [pull requests](docs/agents/ci.md)

## Index

| Doc | Read it before |
|---|---|
| [layout.md](docs/agents/layout.md) | Adding, moving or looking for a file or directory, or deciding which side of the binding a change goes in |
| [building.md](docs/agents/building.md) | Changing the build of the library, the binding or the CUDA plugin, their options or presets, the install test, the Doxygen target or coverage |
| [dependencies.md](docs/agents/dependencies.md) | Adding, bumping or pinning a dependency, or touching `cmake/modules/` |
| [tests.md](docs/agents/tests.md) | Running the suites, adding or moving a test, or adding a test requirement |
| [test-doubles.md](docs/agents/test-doubles.md) | Writing a test that stands in for an interface, or touching a `mock/` directory |
| [code-style.md](docs/agents/code-style.md) | Writing any code or comment |
| [naming.md](docs/agents/naming.md) | Naming a class that implements an interface, or a function of a fixed number of axes |
| [random-numbers.md](docs/agents/random-numbers.md) | Writing or changing an operation that draws random numbers, or a function on keys |
| [operators-and-exports.md](docs/agents/operators-and-exports.md) | Overloading an operator, marking something `REXLIB_API` or changing how the library links |
| [arrays-and-commands.md](docs/agents/arrays-and-commands.md) | Touching the memory of an array, a dispatcher, an allocator or a view over another array |
| [plugins.md](docs/agents/plugins.md) | Touching plugin loading or the backend surface, or writing a plugin |
| [python-binding.md](docs/agents/python-binding.md) | Touching `python/src/` |
| [python-modules.md](docs/agents/python-modules.md) | Adding a submodule of `_binding` or a module of `python/rexlib/` |
| [python-package.md](docs/agents/python-package.md) | Touching `python/rexlib/` |
| [type-stubs.md](docs/agents/type-stubs.md) | Changing a bound signature, or how the stubs are generated |
| [memory-across-the-border.md](docs/agents/memory-across-the-border.md) | Touching the buffer protocol, DLPack or `dlpack_buffer` |
| [cuda-conventions.md](docs/agents/cuda-conventions.md) | Writing or reviewing code of the CUDA plugin |
| [ci.md](docs/agents/ci.md) | Opening a pull request, changing `.github/workflows/` or how wheels are built, or making a release |

The binding's open questions and its settled work waiting to be scheduled are
in [python/DECISIONS.md](python/DECISIONS.md) and
[python/BACKLOG.md](python/BACKLOG.md).
