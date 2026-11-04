# Pull requests and continuous integration

How pull requests are opened, what the workflows do, and how the wheels are
built.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

The binding is built and tested by the library's workflow; see
[the library's CI](../../../docs/agents/ci.md).

## Wheels

The library's workflow builds them: the `wheels` jobs, one per platform,
from the source distribution the `sdist` job builds, so that every platform
also checks that the source distribution builds. cibuildwheel takes its
settings from `pyproject.toml`; the workflow adds only the compiler cache.

A platform gets a wheel for each CPython older than 3.12, one abi3 wheel for
3.12 and every later CPython, and one for each free-threaded interpreter
cibuildwheel builds for, which from cibuildwheel 4 starts at 3.14.
cibuildwheel builds the abi3 one for 3.12, finds it compatible with each
later interpreter, tests it there instead of building again, and checks it
with `abi3audit`. The binding does not declare that it runs without the GIL,
so a free-threaded interpreter turns the GIL on when it imports it.

The wheel carries librexlib itself, put there by the library's install
rules, and every repair tool is told to leave it alone: `auditwheel
--exclude`, `delvewheel --exclude`, `delocate --ignore-missing-dependencies`.
Vendoring it would rename it under a hash-mangled SONAME, which breaks
`find_package` for plugin authors and the sibling directory the plugins are
discovered in. The namelink is excluded from the wheel instead, since a
wheel stores a symlink as another full copy.

musllinux is skipped: NVIDIA ships no CUDA for musl, so the CUDA plugin is
manylinux only and an Alpine install could never reach the GPU backend.
Alpine can still build from the source distribution.

The wheel jobs build with the setting `build-dir=build/{wheel_tag}`, which
is what lets the compiler cache work. scikit-build-core otherwise builds in a
temporary directory of its own, unrelated to the one the archive is unpacked
into, so the sources sit at a different place relative to the build directory
on every run and `CCACHE_BASEDIR` has nothing stable to rewrite against. The
value has to stay relative: scikit-build-core resolves it against the source
directory, and an absolute one would leave the sources where they were.

For the same reason cibuildwheel is given the source distribution unpacked
at the same path of the workspace in every run, not the archive itself, which
it would unpack into a new temporary directory each time. MSVC puts absolute
paths into what the cache hashes, so on Windows nothing an earlier run cached
would match, and the first wheel of every run would compile from scratch.
