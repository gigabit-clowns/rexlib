# Building

How rexlib is configured, built and tested, and what its options do.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
```

With CMake 3.25 or newer the presets do the same: `cmake --preset debug`,
`cmake --build --preset debug`, `ctest --preset debug`, which build into
`build/debug`. There are `release`, `sanitize` (the address and undefined
behaviour sanitizers) and `sanitize-thread` presets too; the two sanitizer
presets are not offered on Windows; they set the sanitizers' options too,
so a test run behaves as CI's. `coverage`, on Linux only, adds coverage
instrumentation, registers the tests per binary and passes valgrind the
options CI uses, for `ctest --test-dir build/coverage -T MemCheck`. `docs`
configures for the Doxygen reference alone and builds `rexlib-docs`.
`lint` registers the static checks: for now `ctest --preset lint` runs
editorconfig-checker, which has to be installed.
Every preset but `docs` turns `REXLIB_WARNINGS_AS_ERRORS` on, and its
test preset fails when it finds no test, runs the tests in a random order and
gives each ten minutes; 3.25 is what lets the fetched dependencies' headers
count as system headers, whose warnings are not rexlib's. Settings of a
machine of one's own, such
as a compiler cache, go in a `CMakeUserPresets.json`, which git ignores.

CMake 3.18 is the minimum. The sources are written to C++14 and reach anything
newer through the `REXLIB_*` feature macros, so that a compiler without it
still builds them; CMake asks for C++17 but leaves
`CMAKE_CXX_STANDARD_REQUIRED` off, so the request is a preference. One
definition does not follow this, `parse_device_index` in
`src/core/hardware/device_index.cpp`, which uses `std::from_chars`. The
public headers do, and `REXLIB_CHECK_HEADERS` holds them to it.

Options:

| Option | Default | Effect |
|---|---|---|
| `REXLIB_INSTALL` | `ON` | Generates the install and export rules |
| `REXLIB_BUILD_TESTING` | `ON` while `BUILD_TESTING` is on, forced `OFF` otherwise | Builds the test suites |
| `REXLIB_BUILD_DOCS` | `OFF` | Adds the Doxygen target `rexlib-docs` |
| `REXLIB_DOCS_USE_README_AS_MAINPAGE` | `ON` | Uses `README.md` as Doxygen's main page |
| `REXLIB_DOCS_STRIP_BADGES_FROM_MAINPAGE` | `ON` | Uses a generated copy of `README.md` without badge blocks as Doxygen's main page |
| `REXLIB_ENABLE_COVERAGE` | `OFF` | Adds coverage instrumentation |
| `REXLIB_REGISTER_TESTS_PER_BINARY` | `OFF` | Registers one CTest entry per test executable instead of one per Catch2 case |
| `REXLIB_CHECK_HEADERS` | `OFF`, `ON` in the presets | Compiles every public header on its own as C++14, 17, 20 and 23 |
| `REXLIB_WARNINGS_AS_ERRORS` | `OFF`, `ON` in the presets | Makes a compiler warning fail rexlib's own targets (`-Werror`, `/WX`) |
| `REXLIB_CHECK_STYLE` | `OFF`, `ON` in `lint` | Adds `rexlib-style`, a test running editorconfig-checker over every file git tracks |
| `REXLIB_BUILD_BENCHMARKS` | `OFF`, and only with `REXLIB_BUILD_TESTING` | Builds `rexlib-benchmarks`, Catch2 benchmarks run by hand and never by CTest |

`REXLIB_BUILD_TESTING` lets a project that embeds rexlib drop its tests without
turning off `BUILD_TESTING` for its own.

With `REXLIB_INSTALL` on, the tests include two labelled `install`: the first
installs the build into a directory of the build tree, and the second builds
`tests/install/consumer`, a C++14 project that finds rexlib with
`find_package` and nothing else, and runs it except on Windows. A header that
is not installed, or a package that does not describe the library, fails
there.

`rexlib-docs` writes the documentation of the public headers under
`docs/doxygen/html` of the build tree. A warning of Doxygen fails it, so a
comment that names a parameter that is not there does not go unnoticed; a
member left undocumented is not a warning. Doxygen leaves out what sits in
a namespace it has no documentation for, so a new namespace is documented in
`docs/doxygen/namespaces.dox`. The macros that declare an operation are given
to Doxygen as the class they declare, in `docs/doxygen/CMakeLists.txt`, so
that the comment above a declaration documents its class.

With `REXLIB_DOCS_USE_README_AS_MAINPAGE=ON`, the main page is taken from
`README.md`, so the front page and the repository overview stay in one place.
With `REXLIB_DOCS_STRIP_BADGES_FROM_MAINPAGE=ON`, Doxygen reads a generated
copy where everything between `<!-- docs-badges:start -->` and
`<!-- docs-badges:end -->` is removed.

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

## Coverage

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
