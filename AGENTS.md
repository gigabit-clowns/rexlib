# Working on rexlib

rexlib is a device-agnostic array computing library with a pluggable backend
architecture. It is built as a generic ND-array framework, although it will 
gain focus on Cryo-Electron Microscopy (CryoEM) image processing algorithms.
Backends (implementations) for particular hardware live in their own repositories
(except for CPU) and are loaded at run time.

Keep this file true. When a change makes something here wrong or missing —
a moved directory, a new convention, a dependency, a workflow — update it in
the same pull request that causes it.

## Layout

| Path | Holds |
|---|---|
| `include/rexlib/` | The public headers, 234 `.hpp` and 26 `.inl` |
| `src/` | The implementation, plus headers that are not public |
| `src/backends/cpu/` | The CPU backend: builders, kernels, loops, plans |
| `src/core/` | Dispatch, layouts, hardware abstraction, plugin loading |
| `src/ops/`, `src/functional/`, `src/em/` | Operation declarations and the functions that reach them |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `cmake/modules/` | One `fetch_*.cmake` per dependency |
| `cmake/config/` | The template for the installed CMake package config |

The same top level groups appear on both sides, `core`, `backends`, `ops`,
`functional` and `em`, but not every directory has a counterpart: a header with
no implementation of its own lives only under `include/`.

## Building

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
```

CMake 3.16 is the minimum. The sources are written to C++14 and reach anything
newer through the `REXLIB_*` feature macros, so that a compiler without it
still builds them; CMake asks for C++17 but leaves
`CMAKE_CXX_STANDARD_REQUIRED` off, so the request is a preference. One
declaration does not follow this, `parse_device_index` in
`core/hardware/device_index.hpp`, which takes a `std::string_view`.

Options, all `OFF` by default:

| Option | Effect |
|---|---|
| `REXLIB_BUILD_DOC` | Builds the Doxygen target |
| `REXLIB_ENABLE_COVERAGE` | Adds coverage instrumentation |
| `REXLIB_REGISTER_TESTS_PER_BINARY` | Registers one CTest entry per test executable instead of one per Catch2 case |

The last one exists because a memory checker pays its start-up on every test
CTest runs. Discovering each case makes it re-analyse the whole binary once per
case, which took the memcheck job past six hours.

Dependencies are fetched, not found: boost, spdlog, half, pocketfft and eigen,
each through its `cmake/modules/fetch_*.cmake`. Boost and spdlog are linked
statically and privately, so the installed package asks only for `Threads` and
an archive of it is self contained.

## Conventions

`.clang-tidy` is the authority on naming and is enforced in CI. In short:
`lower_case` for everything but template parameters, which are `CamelCase`;
private members carry `m_`; macros are `UPPER_CASE` with a `REXLIB_` prefix.

Indentation is tabs. Lines stay within 80 columns; the handful that do not are
the exception rather than the rule.

Declarations go in a `.hpp`, definitions that must be visible go in an `.inl`
the header includes at its end, and everything else goes in a `.cpp`.

### Comments

Say **what**, never **why**. The why belongs in the commit message and the pull
request. And only say the what when the code does not already: if a reader
understands the line without the comment, the comment does not go in.

There is not a single `TODO` in the repository. Unfinished work is an issue,
not a comment.

### Overloaded operators

They are hidden friends: a `friend` declaration inside the class, so that the
operator is reachable only through argument dependent lookup, and both operands
convert alike. The definition goes in the class body too, which is the only
form that also works for templates and for `constexpr`.

The exception is an operator whose body needs a type that is incomplete in the
header — `strided_layout` compares through its implementation, which is only
forward declared — and those keep their definition in the source file. The
`friend` declaration still hides them.

### Exported symbols

The library builds with hidden visibility. `REXLIB_API` marks what crosses the
shared object boundary. On MSVC, a dll-interface class holding a standard
library member warns with C4251, and `REXLIB_STD_MEMBER_INTERFACE` on the line
above silences it for that member alone, leaving the warning active elsewhere.
`REXLIB_STD_BASE_INTERFACE` does the same for C4275 on a base class.

An operator defined in a class body is implicitly inline and needs no
`REXLIB_API`.

## Plugins

A plugin is a shared object exporting `rexlib_get_plugin`, returning a
`const plugin*`. `plugin_loader` resolves that symbol by name, so the contract
is the symbol and the `plugin` interface, not the C++ ABI of the loader.

It is still a C++ ABI boundary for everything the plugin touches: the plugin
and the core have to agree on compiler, standard library, and on Windows the
runtime and whether it is the debug one.

## Continuous integration

| Workflow | Does |
|---|---|
| `build-and-test.yml` | Builds and tests the matrix, then the SonarQube scan |
| `deploy.yml` | Builds the documentation, and deploys it on a version tag |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml` | Returns Actions cache space |

The matrix covers Linux, macOS and Windows, gcc, clang and MSVC, x86_64 and
Arm. MSVC builds through Ninja rather than the Visual Studio generator, since
that is the only way a compiler cache can be used, and the environment is set
up with `vswhere` and `vcvars` beforehand.

Every entry runs the suites, and the `ubuntu-latest` ones run them under a
memory checker with `REXLIB_REGISTER_TESTS_PER_BINARY`. The rest run them
plainly: the valgrind of `ubuntu-22.04` predates DWARF 5 and gives up on what
clang emits there, and macOS and Windows have no checker set up. A leak
therefore fails the pull request rather than being found afterwards.

Compilation is cached with ccache. The Actions cache is 10 GB for the whole
repository while access is per branch, so entries pile up faster than they look
like they should; `clean-up-caches.yml` drops what a newer run replaced, and
everything belonging to a pull request when it closes.
