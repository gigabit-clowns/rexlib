# Working on rexlib

rexlib is a device-agnostic array computing library with a pluggable backend
architecture. It is built as a generic ND-array framework, although it will 
gain focus on Cryo-Electron Microscopy (CryoEM) image processing algorithms.
Backends (implementations) for particular hardware live in their own repositories
(except for CPU) and are loaded at run time.

This file is a map; the detail lives in `docs/agents/`. Keep both true. When a
change makes something in them wrong or missing — a moved directory, a new
convention, a dependency, a workflow — update them in the same pull request
that causes it.

## Building

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
```

## Always

- C++14 in the sources and public headers; anything newer goes through the
  `REXLIB_*` feature macros. [building](docs/agents/building.md)
- Every source file, the CMake files included, starts with
  `SPDX-License-Identifier: GPL-3.0-only`. [code style](docs/agents/code-style.md)
- Tabs, 80 columns, naming as `.clang-tidy` says. [code style](docs/agents/code-style.md)
- Declarations in a `.hpp`, visible definitions in an `.inl`, the rest in a
  `.cpp`. [code style](docs/agents/code-style.md)
- Comments say what, never why, and there is no `TODO`. [code style](docs/agents/code-style.md)
- A new `.cpp` is named in the `CMakeLists.txt` of its directory.
  [layout](docs/agents/layout.md)
- Test doubles are trompeloeil mocks. [test doubles](docs/agents/test-doubles.md)
- A pull request is assigned to whoever opens it and labelled.
  [pull requests](docs/agents/ci.md)

## Index

| Doc | Read it before |
|---|---|
| [layout.md](docs/agents/layout.md) | Adding, moving or looking for a file or directory |
| [building.md](docs/agents/building.md) | Changing the build, its options, the presets, the install test, the Doxygen target or coverage |
| [dependencies.md](docs/agents/dependencies.md) | Adding, bumping or pinning a dependency, or touching `cmake/modules/` |
| [code-style.md](docs/agents/code-style.md) | Writing any code or comment |
| [naming.md](docs/agents/naming.md) | Naming a class that implements an interface, or a function of a fixed number of axes |
| [random-numbers.md](docs/agents/random-numbers.md) | Writing or changing an operation that draws random numbers, or a function on keys |
| [test-doubles.md](docs/agents/test-doubles.md) | Writing a test that stands in for an interface, or touching a `mock/` directory |
| [operators-and-exports.md](docs/agents/operators-and-exports.md) | Overloading an operator, marking something `REXLIB_API` or changing how the library links |
| [arrays-and-commands.md](docs/agents/arrays-and-commands.md) | Touching the memory of an array, a dispatcher, an allocator or a view over another array |
| [plugins.md](docs/agents/plugins.md) | Touching plugin loading or the boundary with a backend plugin |
| [ci.md](docs/agents/ci.md) | Opening a pull request, or changing `.github/workflows/` |
