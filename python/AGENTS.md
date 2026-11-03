# Working on rexlib-python

rexlib-python is the Python binding for [rexlib](https://github.com/gigabit-clowns/rexlib),
a device-agnostic array computing library with a pluggable backend
architecture. It is a binding and nothing more: every computation happens in
C++, and this repository decides only what the Python side of that API looks
like.

This file is a map. The detail is in [docs/agents/](docs/agents/), one file
per theme. Keep both true. When a change makes something in them wrong or
missing — a moved directory, a new convention, a dependency, a workflow —
update it in the same pull request that causes it.

## Building and testing

```
cmake -S external/rexlib -B build/rexlib -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=~/.local -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF
cmake --build build/rexlib --target install --parallel $(nproc)

pip install --group dev
CMAKE_PREFIX_PATH=~/.local CMAKE_BUILD_PARALLEL_LEVEL=$(nproc) \
  pip install . --no-build-isolation -v -Ccmake.define.REXLIB_PYTHON_BUILD_TESTING=ON

./scripts/run-tests.sh
```

## Always

- A change that could be expressed in C++ or in Python goes in
  `python/rexlib/`. See [layout](docs/agents/layout.md).
- The binding mirrors the C++ API and decides nothing. See
  [binding](docs/agents/binding.md).
- Every bound entity gets a `.hpp` and a `.cpp`, with a `declare_` and a
  `define_` function. See [binding](docs/agents/binding.md).
- Tabs, 80 columns, on both sides. See [package](docs/agents/package.md).
- Every source file, the CMake files included, starts with
  `SPDX-License-Identifier: GPL-3.0-only`. See
  [package](docs/agents/package.md).
- A new `.cpp` is named in the `CMakeLists.txt` of its directory. See
  [building](docs/agents/building.md).
- Comments say what, never why, and there is no `TODO`. See
  [comments](docs/agents/comments.md).
- A change is tested from Python, in the directory of the module whose
  surface it exercises. See [tests](docs/agents/tests.md).
- `ruff check .` passes. See [package](docs/agents/package.md).
- Whoever opens a pull request assigns it to themselves and labels it. See
  [ci](docs/agents/ci.md).

## Index

| Doc | Read it before |
|---|---|
| [layout](docs/agents/layout.md) | Adding a file or directory, or deciding which side a change goes in |
| [modules](docs/agents/modules.md) | Adding a submodule of `_binding` or a module of `python/rexlib/` |
| [building](docs/agents/building.md) | Building in any other way than above, or changing `CMakeLists.txt` or its options |
| [type-stubs](docs/agents/type-stubs.md) | Changing a bound signature, or how stubs are generated or collected |
| [binding](docs/agents/binding.md) | Touching `src/` |
| [memory-across-the-border](docs/agents/memory-across-the-border.md) | Touching the buffer protocol, DLPack or `dlpack_buffer` |
| [package](docs/agents/package.md) | Touching `python/rexlib/` |
| [comments](docs/agents/comments.md) | Writing a comment |
| [tests](docs/agents/tests.md) | Adding or moving a test, or adding a test requirement |
| [ci](docs/agents/ci.md) | Opening a pull request, or touching `.github/workflows/` or how wheels are built |
