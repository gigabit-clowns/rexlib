# Pull requests and continuous integration

How pull requests are opened, what the workflows do, and how the wheels are
built.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

| Workflow | Does |
|---|---|
| `build-and-test.yml` | Builds with CMake and with pip across the matrix, runs ruff and the suites, then the SonarQube scan |
| `deploy.yml` | Generates the stubs, builds the wheels and the source distribution, and publishes them |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml`, `keep-caches-warm.yml` | Manage the Actions cache |

`build_with_cmake` covers Linux with gcc and clang, macOS, and Windows on both
architectures; it exists to keep the plain CMake path working and to produce
the compilation database the scanner reads. `build_with_pip` is the one that
runs the tests, across five platforms and Python 3.10 through 3.15, minus
Windows ARM on 3.10, which CPython never shipped there.

Both build rexlib first, into a prefix outside the project, and point the
build at it with `rexlib_ROOT`. It is the bulk of the wall clock, and ccache
is what makes every run after the first a link. The compiler cache and the
fetched sources are saved only on `main`: a pull request reads them and writes
nothing, so one branch cannot evict another's.

Every job moves the submodule to rexlib's `main` before building. That step is
temporary and goes away, along with the Renovate rule holding the submodule
back, once rexlib tags releases.

Coverage is produced by one entry only, Linux on 3.15, and handed to the scan
as an artifact.

## Wheels

`deploy.yml` generates the stubs once, in a job of its own, and hands them to
every wheel — a cross-compiled wheel cannot make its own.

A platform gets a wheel for 3.10, one for 3.11, one abi3 wheel for 3.12 and
every later CPython, and one for each free-threaded interpreter. cibuildwheel
builds the abi3 one for 3.12, finds it compatible with each later interpreter,
tests it there instead of building again, and checks it with `abi3audit`. The
macOS x86_64 wheels target 10.14, the oldest nanobind supports. The wheels then
build rexlib from `external/rexlib` rather than being pointed at one, so that
cibuildwheel's architecture, deployment target and SDK apply to it too instead
of having to be repeated to a step outside.

The wheel carries librexlib itself, put there by this project's install rules,
and every repair tool is told to leave it alone: `auditwheel --exclude`,
`delvewheel --exclude`, `delocate --ignore-missing-dependencies`. Vendoring it
would rename it under a hash-mangled SONAME, which breaks `find_package` for
plugin authors and the sibling directory the plugins are discovered in. The
namelink is excluded from the wheel instead, since a wheel stores a symlink as
another full copy.

musllinux is skipped: NVIDIA ships no CUDA for musl, so rexlib-cuda is
manylinux only and an Alpine install could never reach the GPU backend. Alpine
can still build from the source distribution.

`scripts/check_wheels_agree.py` asserts that every wheel of a platform carries
the same rexlib, which is what makes reusing rexlib's objects between
interpreters sound rather than assumed.

The source distribution names `external/rexlib/**` explicitly, because
`git ls-files` does not report a submodule's contents and the archive would
otherwise carry an empty directory and fail to build.

The job that installs that distribution sets `SKBUILD_BUILD_DIR`, which is
what lets its compiler cache work. scikit-build-core otherwise builds in a
temporary directory of its own, unrelated to the one pip unpacked the archive
into, so the sources sit at a different place relative to the build directory
on every run and `CCACHE_BASEDIR` has nothing stable to rewrite against. The
value has to stay relative — scikit-build-core resolves it against the source
directory, and an absolute one would leave the sources where they were.

Publishing to PyPI is wired up but disabled. Every push to `main` updates the
`development` pre-release with its binaries.
