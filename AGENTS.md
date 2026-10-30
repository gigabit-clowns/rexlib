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
| `include/rexlib/` | The public headers |
| `src/` | The implementation, plus headers that are not public |
| `src/backends/cpu/` | The CPU backend: builders, kernels, loops, plans |
| `src/core/` | What an operation is and which operands it admits (`operation/`, with the shape and type rule languages in `shape_rule/` and `type_rule/`), how it runs (`dispatch/`), layouts, hardware abstraction, plugin loading |
| `src/ops/`, `src/functional/` | Operation declarations and the functions that reach them |
| `src/em/` | The CryoEM operations and the functions that reach them, in namespace `rexlib::em`. Their CPU programs live with the others, under `src/backends/cpu/builders/em/` |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `cmake/modules/` | One `rexlib_fetch_*.cmake` per dependency, plus the `Find*.cmake` for those that ship no package config |
| `cmake/config/` | The template for the installed CMake package config |

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
| `REXLIB_BUILD_TESTING` | `ON` while `BUILD_TESTING` is on, forced `OFF` otherwise | Builds the test suites |
| `REXLIB_BUILD_DOCS` | `OFF` | Adds the Doxygen target `rexlib-docs` |
| `REXLIB_ENABLE_COVERAGE` | `OFF` | Adds coverage instrumentation |
| `REXLIB_REGISTER_TESTS_PER_BINARY` | `OFF` | Registers one CTest entry per test executable instead of one per Catch2 case |

`REXLIB_BUILD_TESTING` lets a project that embeds rexlib drop its tests without
turning off `BUILD_TESTING` for its own.

`rexlib-docs` writes the documentation of the public headers under
`doc/doxygen/html` of the build tree. A warning of Doxygen fails it, so a
comment that names a parameter that is not there does not go unnoticed; a
member left undocumented is not a warning. Doxygen leaves out what sits in
a namespace it has no documentation for, so a new namespace is documented in
`doc/doxygen/namespaces.dox`. The macros that declare an operation are given
to Doxygen as the class they declare, in `doc/doxygen/CMakeLists.txt`, so
that the comment above a declaration documents its class.

Each suite carries its name as a CTest label, so `ctest -L unitary` and
`ctest -L integration` run one of them. The label is set whichever way the
cases were registered. Under CMake 3.18 each suite is a single test, since
listing the cases needs the JSON support of CMake 3.19.

On Windows the library and the test executables are built into one
directory, `bin/` of the build tree, unless `CMAKE_RUNTIME_OUTPUT_DIRECTORY`
is given: Windows finds a library beside the executable that loads it, and
a plugin a test loads finds `rexlib.dll` there too.

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

Every dependency is fetched and built with rexlib by default, static, and
ends up inside its library. Turning `REXLIB_BUILD_DEPENDENCIES` off finds
them all installed instead, with `find_package`, as a distribution builds
rexlib. Turning off `REXLIB_BUILD_<DEP>` (`BOOST`, `SPDLOG`, `HALF`,
`POCKETFFT`, `EIGEN`, and for the tests `CATCH2` and `TROMPELOEIL`) does that
for one of them. `cmake_dependent_option` declares those toggles, so that the
global one forces the others off.

A dependency is a branch where it is used, in `CMakeLists.txt` for the
library and in `tests/CMakeLists.txt` for the tests. Built, it is a call to
`rexlib_fetch_<dep>()` with what pins the release it fetches. Found, it is
the `find_package` call, with the oldest version that works. Each
`rexlib_fetch_<dep>()` has a module of its own in `cmake/modules/`, reads
nothing but its arguments, and sets the options of the project it fetches in
its own scope. A new dependency gets a toggle, a branch, a module and a
manager in `renovate.json`.

Those calls hold the only pins, in forms Renovate keeps up to date. What is
released on GitHub or GitLab is pinned by the commit of its release tag,
`COMMIT <sha>) # <tag>`, as the actions of the workflows are; pocketfft,
which makes no releases, by a commit of its `cpp` branch, `# cpp`. Boost is
pinned by its tag and the SHA256 of the archive it attaches to the release,
since the archive of a commit lacks its libraries, which are submodules, and
half by its version and the SHA256 of its archive on SourceForge. Renovate
bumps the versions, not the hashes, so such a bump fails to configure until
the hash CMake reports is pasted in. `FETCHCONTENT_SOURCE_DIR_<NAME>` points a
fetch at a local checkout.

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

Every source file, the CMake files included, starts with
`SPDX-License-Identifier: GPL-3.0-only`.

Declarations go in a `.hpp`, definitions that must be visible go in an `.inl`
the header includes at its end, and everything else goes in a `.cpp`.

### Implementations of an interface

A class that implements an interface, or narrows one for a backend, is named
`<qualifier>_<interface>`, the qualifier saying which one it is: a backend, a
strategy, a kind of memory. `cpu_device` and `cpu_command_queue` implement
`device` and `command_queue`, `cpu_program` narrows `program` for the CPU
backend, and `host_buffer` and `eager_dispatcher` implement `buffer` and
`dispatcher`. Its files carry the same name.

The name stands on its own, without the namespace around it. A backend keeps
its code in a namespace of its own, `rexlib::cpu` here and `rexlib::cuda` in
[rexlib-cuda](https://github.com/gigabit-clowns/rexlib-cuda), so that its
helpers cannot clash with those of another backend loaded into the same
process; that namespace is never what tells an implementation from its
interface. So the CPU device is `rexlib::cpu::cpu_device`, and inside the
backend a plain `device` is the interface.

### Functions of a fixed number of axes

A function acting along the last two or three axes is the operation that
acts along named axes, with the axes filled in. An established NumPy name
keeps NumPy's spelling, as `fft2` and `fft3` do; any other function takes a
`_2d` or `_3d` suffix, as `standardize_background_2d` does.

### Comments

Say **what**, never **why**. The why belongs in the commit message and the pull
request. And only say the what when the code does not already: if a reader
understands the line without the comment, the comment does not go in.

There is not a single `TODO` in the repository. Unfinished work is an issue,
not a comment.

### Test doubles

A double is a trompeloeil mock, kept in the `mock/` directory beside the
tests of its area, or in the test file when only that file uses it. It states
the calls it expects where they happen, and a check on an argument that only
lives during the call goes in the expectation's `WITH`. A double does not copy
what it saw into a log or a counter for the test to read back afterwards.

trompeloeil cannot mock `operator()` or a template, so a callable the code
under test takes by value is a small copyable forwarder onto a mock with
concrete signatures. A mocked call holds trompeloeil's global lock, so it must
never wait for another mocked call.

Hand written types remain where they are not doubles of an interface: types a
template under test is instantiated with or derived from, test inputs, and a
double the code under test constructs itself before a test can reach it.

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

The dependencies linked as static libraries do not follow that visibility on
their own: a static library exports whatever it defines. On Linux the shared
library is therefore linked with `--exclude-libs,ALL`, so that boost stays
inside it rather than being resolved against another copy the process holds.

### Arrays and the commands that access them

A queue runs a command after the commands it depends on and promises no other
order, so two commands that touch the same memory have to be ordered by a
dependency. Arrays keep the record that makes this automatic: every array
carries an `access_hazard_tracker` holding the tokens of the commands that
still write or read its memory. A dispatcher takes the dependencies of a
command from the trackers of its operands, and records the token it gets back
on them.

Three rules follow for anything that touches the memory of an array:

- An array over the memory of another is built from that array, through the
  constructor of `array_implementation` that takes one, never from
  `share_storage()`. That is what makes the two share a tracker; `subarray`
  and the complex views do it. Two arrays built separately over one buffer are
  not ordered against each other.
- The host reaches an array through `get_host_data`, which waits for the
  commands in its way. `buffer::get_host_ptr` waits for nothing.
- Memory an allocator hands out comes with a token. Whoever allocates gives it
  to the tracker of the new array, or to the dependencies of the command when
  the buffer belongs to no array.

## Plugins

A plugin is a shared object exporting `rexlib_get_plugin`, returning a
`const plugin*`. `plugin_loader` resolves that symbol by name, so the contract
is the symbol and the `plugin` interface, not the C++ ABI of the loader.

It is still a C++ ABI boundary for everything the plugin touches: the plugin
and the core have to agree on compiler, standard library, and on Windows the
runtime and whether it is the debug one.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

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
with `REXLIB_BUILD_DEPENDENCIES` off, so that every `find_package` branch
keeps being exercised. It takes boost, eigen, spdlog, catch2 and trompeloeil
from apt and puts the half and pocketfft headers on the include path the way a
packager would, which is what covers `Findhalf.cmake` and
`Findpocketfft.cmake`. The pins and the hash of half it reads from
`CMakeLists.txt`. The job also asserts that no dependency reached the install
tree.

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
