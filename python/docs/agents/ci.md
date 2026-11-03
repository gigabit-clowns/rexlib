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

No workflow builds wheels yet. These are the decisions the wheels of the
binding's own repository were built on, which the wheels of this one keep.

A platform gets a wheel for each CPython older than 3.12, one abi3 wheel for
3.12 and every later CPython, and one for each free-threaded interpreter.
cibuildwheel builds the abi3 one for 3.12, finds it compatible with each
later interpreter, tests it there instead of building again, and checks it
with `abi3audit`.

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

A job that installs the source distribution sets `SKBUILD_BUILD_DIR`, which
is what lets its compiler cache work. scikit-build-core otherwise builds in a
temporary directory of its own, unrelated to the one pip unpacked the archive
into, so the sources sit at a different place relative to the build directory
on every run and `CCACHE_BASEDIR` has nothing stable to rewrite against. The
value has to stay relative: scikit-build-core resolves it against the source
directory, and an absolute one would leave the sources where they were.
