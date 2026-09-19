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
| `include/rexlib/` | The public headers, 268 `.hpp` and 26 `.inl` |
| `src/` | The implementation, plus headers that are not public |
| `src/backends/cpu/` | The CPU backend: builders, kernels, loops, plans |
| `src/core/` | Dispatch, layouts, hardware abstraction, plugin loading |
| `src/ops/`, `src/functional/`, `src/em/` | Operation declarations and the functions that reach them |
| `src/em/image/` | The image I/O subsystem, one directory per file format |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `cmake/modules/` | One `rexlib_add_*.cmake` per dependency, plus the `Find*.cmake` for those that ship no package config |
| `cmake/config/` | The template for the installed CMake package config |

The same top level groups appear on both sides, `core`, `backends`, `ops`,
`functional` and `em`, but not every directory has a counterpart: a header with
no implementation of its own lives only under `include/`.

Every directory holding sources carries a `CMakeLists.txt` naming them, which
adds them to the component target its group belongs to and descends into the
directories below it. A new `.cpp` has to be named there before it is built:
the lists are explicit rather than globbed, so that adding a file cannot go
unnoticed by the build system and so that link order does not follow whatever
order the file system returns. That order matters here, since the CPU program
builders register themselves through objects at namespace scope.

## Building

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
```

CMake 3.18 is the minimum. The sources are written to C++14 and reach anything
newer through the `REXLIB_*` feature macros, so that a compiler without it
still builds them; CMake asks for C++17 but leaves
`CMAKE_CXX_STANDARD_REQUIRED` off, so the request is a preference. One
declaration does not follow this, `parse_device_index` in
`core/hardware/device_index.hpp`, which takes a `std::string_view`.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_INSTALL` | `ON` | Generates the install and export rules |
| `REXLIB_BUILD_TESTING` | `BUILD_TESTING` | Builds the test suites |
| `REXLIB_BUILD_DOC` | `OFF` | Builds the Doxygen target |
| `REXLIB_ENABLE_COVERAGE` | `OFF` | Adds coverage instrumentation |
| `REXLIB_REGISTER_TESTS_PER_BINARY` | `OFF` | Registers one CTest entry per test executable instead of one per Catch2 case |

`REXLIB_BUILD_TESTING` lets a project that embeds rexlib drop its tests without
turning off `BUILD_TESTING` for its own.

Each suite carries its name as a CTest label, so `ctest -L unitary` and
`ctest -L integration` run one of them. The label is set whichever way the
cases were registered.

The last one exists because a memory checker pays its start-up on every test
CTest runs. Discovering each case makes it re-analyse the whole binary once per
case, which took the memcheck job past six hours.

`REXLIB_ENABLE_COVERAGE` adds `-pthread` beside `--coverage` under GCC. The
counters gcov keeps are one per arc and shared by every thread running the
code, and the CPU backend runs its loops on a thread pool, so updated the
ordinary way the increments race and are lost. That does not merely
undercount: gcov measures a subset of the arcs and derives the rest by
conservation of flow, so counters that disagree make a derived branch come out
negative, and gcovr stops at the first one. The GCC driver selects
`-fprofile-update=prefer-atomic` when `-pthread` is on the compile line, which
is the whole reason the flag is named there; nothing else sets it, since
`THREADS_PREFER_PTHREAD_FLAG` is unset and `Threads::Threads` only links.

Naming `-fprofile-update=atomic` directly would say the same thing and is what
the option is for, but ccache learned that option in 4.10 and treats it as
unsupported before then, making every instrumented unit uncacheable. The
runners carry 4.5.1 on jammy, 4.9.1 on noble and on Windows, so all but macOS
would recompile the whole library on every run. `-pthread` is an ordinary
flag every version caches.

Clang ignores `-pthread` here — its driver acts on `-fprofile-update` alone —
so its counters are not atomic and its counts are lost the same way. It is
left as it is because clang selects arcs differently and has never produced a
negative, and because buying atomic counters there costs the cache.

Dependencies come through one `cmake/modules/rexlib_add_*.cmake` each: boost,
spdlog, half, pocketfft and eigen for the library, catch2 and trompeloeil for
the tests. Every one is fetched and built by default and can instead be taken
from the system, either all at once with `REXLIB_USE_SYSTEM_DEPENDENCIES` or
one at a time with `REXLIB_USE_SYSTEM_BOOST` and its siblings, which default to
the value the global one had when the build directory was first configured.

Each function takes the release to fetch as `VERSION` and, separately, the one
`find_package` demands as `MINIMUM`. The fetched pin tracks the newest release
and would be a floor no distribution meets. Renovate reads `VERSION` in place,
so it stays a literal in `CMakeLists.txt`. `FETCHCONTENT_SOURCE_DIR_<NAME>`
points a fetch at a local checkout without either option.

half and pocketfft ship no CMake package config anywhere, so their system path
goes through this project's own `Findhalf.cmake` and `Findpocketfft.cmake`.
Neither header carries a version, so neither module can check one.

Every dependency is private and none appears in a public header, so the
installed package asks only for `Threads` whichever way they were obtained,
and an archive of a default build is self contained.

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
| `build-and-test.yml` | Builds and tests the matrix and a system dependency build, then the SonarQube scan |
| `deploy.yml` | Builds the documentation, and the binary archives, and publishes both |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml` | Returns Actions cache space |

The matrix covers Linux, macOS and Windows, gcc, clang and MSVC, x86_64 and
Arm. MSVC builds through Ninja rather than the Visual Studio generator, since
that is the only way a compiler cache can be used, and the environment is set
up with `vswhere` and `vcvars` beforehand.

Beside the matrix, one `ubuntu-latest` job builds against system dependencies,
so that the `find_package` half of every `rexlib_add_*` module keeps being
exercised. It takes boost, eigen, spdlog and catch2 from apt and puts the half
and pocketfft headers on the include path the way a packager would, which is
what covers `Findhalf.cmake` and `Findpocketfft.cmake`; trompeloeil, packaged
nowhere and more than one header, stays fetched. The job also asserts that no
dependency reached the install tree.

Every entry runs the suites, and the `ubuntu-latest` ones run them under a
memory checker with `REXLIB_REGISTER_TESTS_PER_BINARY`. The rest run them
plainly: the valgrind of `ubuntu-22.04` predates DWARF 5 and gives up on what
clang emits there, and macOS and Windows have no checker set up. A leak
therefore fails the pull request rather than being found afterwards.

Compilation is cached with ccache. The Actions cache is 10 GB for the whole
repository while access is per branch, so entries pile up faster than they look
like they should; `clean-up-caches.yml` drops what a newer run replaced, and
everything belonging to a pull request when it closes.

The scan runs in a job of its own, so the `ubuntu-latest` gcc entry hands it
what it reads as an artifact. That is `compile_commands.json` and the include
directories named in it, which are the headers of the fetched dependencies;
the object files and the test binaries are the bulk of a build folder and the
scanner opens none of them. The list is derived from the compilation database
rather than written down, so a new dependency needs no change here.

### Binary archives

`deploy.yml` publishes an archive per platform, so that the continuous
integration of a plugin unpacks the library instead of building it. They are
built Debug: their only reader is a test run, and Debug is what leaves the
`REXLIB_ASSERT` in `src/` compiled in, where Release drops them with `NDEBUG`.
The cost is size, a Debug archive being an order of magnitude larger.

Windows publishes both configurations, because there the plugin has to match:
`/MD` and `/MDd` disagree on `_ITERATOR_DEBUG_LEVEL` and on the container
layouts that follow from it. Linux and macOS publish only Debug, since a
Release plugin links a Debug library there without trouble.

They are not for users, who build from source. Nothing here replaces the
`manylinux` guarantees the wheels used to carry.
