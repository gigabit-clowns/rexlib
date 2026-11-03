# Layout

Where the code, the tests and the build files of rexlib live.

| Path | Holds |
|---|---|
| `include/rexlib/` | The public headers |
| `src/` | The implementation, plus headers that are not public |
| `src/backends/cpu/` | The CPU backend: builders, kernels, loops, plans |
| `src/core/` | What an operation is and which operands it admits (`operation/`, with the shape and type rule languages in `shape_rule/` and `type_rule/`), how it runs (`dispatch/`), layouts, hardware abstraction, plugin loading |
| `src/ops/`, `src/functional/` | Operation declarations and the functions that reach them |
| `src/em/` | The CryoEM operations and the functions that reach them, in namespace `rexlib::em`. Their CPU programs live with the others, under `src/backends/cpu/builders/em/` |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `tests/headers/` | The check that compiles each public header on its own, under every C++ standard the compiler has |
| `tests/install/` | The test that installs rexlib and builds `consumer/`, a project of its own, against the install |
| `benchmarks/` | Catch2 benchmarks, mirroring the layout of `src/`, which reuse the fixtures of `tests/unitary/` |
| `cmake/modules/` | One `rexlib_fetch_*.cmake` per dependency, plus the `Find*.cmake` for those that ship no package config |
| `cmake/config/` | The template for the installed CMake package config |
| `docs/doxygen/` | The reference of the C++ headers, with Doxygen |
| `docs/templates/source/` | Templates of a header, an inline file and a source |
| `docs/agents/` | The docs that `AGENTS.md` indexes |

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
builders register themselves through objects at namespace scope.
