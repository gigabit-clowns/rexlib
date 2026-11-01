# Working on rexlib-cuda

rexlib-cuda is the CUDA hardware backend of
[rexlib](https://github.com/gigabit-clowns/rexlib). It is a plugin: a shared
object that rexlib's plugin manager loads at run time, which registers a CUDA
`device_backend` with the device manager. It has no public headers and no
interface of its own beyond the one rexlib defines.

This file is a map. The detail is in the docs under `docs/agents/`, listed
below. Keep them true. When a change makes something in this file or in those
docs wrong or missing — a moved directory, a new convention, a dependency, a
workflow — update it in the same pull request that causes it.

## Build and test

```
git submodule update --init
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The presets need CMake 3.21 and a `rexlib_ROOT` that reaches an installed
rexlib. [building.md](docs/agents/building.md) says how to build one from the
submodule.

## Always

- C++20, required rather than preferred. ([building](docs/agents/building.md))
- Every source starts with `// SPDX-License-Identifier: GPL-3.0-only`, and
  every CMake file with `# SPDX-License-Identifier: GPL-3.0-only`.
  ([conventions](docs/agents/conventions.md))
- Tabs, 80 columns. ([conventions](docs/agents/conventions.md))
- Declarations in a `.hpp`, visible definitions in an `.inl`, the rest in a
  `.cpp`. ([conventions](docs/agents/conventions.md))
- Every call into the CUDA runtime goes through a `REXLIB_CUDA_CHECK*` macro.
  ([conventions](docs/agents/conventions.md))
- A new `.cpp` is named in the `CMakeLists.txt` of its directory.
  ([building](docs/agents/building.md))
- A new dependency gets a toggle, a branch, a module and a manager in
  `renovate.json`. ([dependencies](docs/agents/dependencies.md))
- Everything is in `namespace rexlib::cuda`.
  ([layout](docs/agents/layout.md))
- Whoever opens a pull request assigns it to themselves and labels it.
  ([ci](docs/agents/ci.md))

## Docs

| Doc | Read it before |
|---|---|
| [layout.md](docs/agents/layout.md) | Looking for where something lives, or adding a file or directory |
| [building.md](docs/agents/building.md) | Configuring or building, changing a preset, an option or a target, or choosing CUDA architectures or a host compiler |
| [dependencies.md](docs/agents/dependencies.md) | Adding, bumping or pinning a dependency, or touching `renovate.json` |
| [conventions.md](docs/agents/conventions.md) | Writing or reviewing any C++ source |
| [tests.md](docs/agents/tests.md) | Running the suites, adding a test, or debugging a test that skips or fails on one platform |
| [ci.md](docs/agents/ci.md) | Opening a pull request, changing a workflow, or building and publishing wheels |
