# Pull requests, continuous integration and releases

How a pull request is opened and what the workflows check.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

`.github/workflows/ci.yml` runs on every pull request and every push to
`main`, with no path filter: a change runs every job it could break. The jobs
run the preset commands a developer runs. They use public actions only, on
runner images named by version rather than `-latest`, and check the
repository out without keeping its credentials. Every job has a timeout,
and a newer push to a pull request cancels the run it supersedes; runs on
`main` are never cancelled.

A workflow names each action by its full version, and
`.github/workflows/actions.lock` locks that version, and what the action
itself uses, to a commit, which is what the runner runs. The lockfile is
written by `gh actions-lock`, an extension of the GitHub CLI
(`gh extension install github/gh-actions-lock`), and never by hand: a change
that adds an action or moves one to another version runs it again and
commits both files.

| Job | What it checks |
|---|---|
| One per platform and toolchain | Linux x86_64 with gcc and with clang, Linux aarch64 with gcc, macOS arm64, Windows x86_64 and arm64 with MSVC, Windows x86_64 with MinGW. Each builds the `debug` preset with warnings as errors and runs every suite, the header check and the install test. All but MinGW also build the Python binding, with the newest Python, and run its tests. The two Linux rows also install the library without its debug information and keep that install as an artifact |
| Address and undefined behaviour sanitizers; thread sanitizer | The `sanitize` and `sanitize-thread` presets, built with clang. These are the default build: the dependencies are fetched and built with rexlib, and warnings do not fail it |
| CMake 3.18 | The one job without presets: CMake 3.18.0, Release with warnings as errors, and the suites. Then the library is installed, and the Python binding is built alone against that install, with the oldest Python it supports, and tested |
| CUDA 12, Linux x86_64; CUDA 13, Linux aarch64 | The CUDA plugin, built as a third party builds a plugin: against the library the Linux row of its architecture installed, on a newer image and with another compiler, with warnings as errors, and tested. These jobs build no library of their own and keep no compiler cache. The plugin's own dependencies are installed by vcpkg. The toolkit's compiler and runtime come from NVIDIA's Ubuntu repository, pinned to the minor version. The CUDA 12 job builds the plugin with CMake 3.18.0. The runners have no GPU, so the tests that need one skip themselves |
| Source distribution | `uv build --sdist`, kept for the wheel jobs to build from |
| Wheels: Linux x86_64 and aarch64, macOS arm64, Windows x86_64 and arm64 | cibuildwheel builds the `rexlib` wheels from the source distribution, as `pyproject.toml` sets it up: one per CPython before 3.12, one abi3 wheel for 3.12 and every later CPython, which abi3audit checks, and one per free-threaded CPython. Each is imported on every interpreter it serves. The compiler cache carries the library from one interpreter's build to the next; on Linux, where the wheels are built in a container, the container runs the runner's ccache from `/host`. Each job then builds the install test's sample plugin as a wheel against the abi3 wheel, with Python 3.14 and without build isolation, and checks that the default catalog loads it |
| CUDA 12 and 13 wheels, Linux x86_64 and aarch64 | cibuildwheel builds the CUDA plugin's wheel of each CUDA major, as `plugins/cuda/cuda12/` and `cuda13/` set it up, against the rexlib wheels of the run, and checks that the default catalog loads the plugin, which needs no GPU |
| ABI report, on pushes to `main` only | Report only: abidiff compares the exported symbols of librexlib in the Linux x86_64 abi3 wheel with those of the latest release's, in the job summary. It skips itself until a release carries that wheel, never fails, and the gate does not wait for it. The wheels carry no debug information, so a change of a class's layout goes unseen |
| Documentation | The `docs` preset; a Doxygen warning fails it |
| Lint | `gh actions-lock --verify`, which fails when the lockfile misses an action or a locked commit no longer verifies; then the `lint` preset, with the `lint` group installed: clang-tidy on the library's sources as they compile, every file git tracks against `.editorconfig`, and the Python files against ruff. The dependencies are installed, as in the jobs with warnings as errors |
| All checks | Fails unless every other job succeeded |

The rows hold the oldest gcc and clang of the supported Ubuntu LTS images; the
sanitizer jobs build with the newest clang.

Every job with warnings as errors, and Lint, builds against installed
dependencies, whose headers count as system headers, so that only rexlib's
warnings and findings fail it. vcpkg installs them into a prefix at the baseline
`.github/vcpkg/vcpkg.json` names, for the row's triplet (static libraries,
with the dynamic C runtime on Windows), and the `find_package` branch of each
finds them, as a packager's would.

The Linux gcc row builds the `coverage` preset instead of `debug`. It runs the
suites under valgrind with `ctest --test-dir build/coverage -T MemCheck`: the
preset registers one test per binary and gives valgrind the options that make
a leak of any kind, or any other error it reports, fail the test, except what
`tests/valgrind.supp` suppresses: the dynamic linker's records of a library
still loaded at exit, which every plugin a `plugin_manager` accepted is. Then
gcovr, from the `coverage` group, measures the coverage, and the SonarQube
scan reads it with the compilation database of the same build.

A row is named by platform and toolchain, never by compiler version, which
is the runner image's to choose. A `run` step reads the runner's values from
the environment (`$RUNNER_TEMP`); an action's `with`, which no shell expands,
reads them from the context (`${{ runner.temp }}`).

MSVC builds through Ninja, which is what lets ccache cache it, with its debug
information embedded in each object for the same reason;
`ilammy/msvc-dev-cmd` sets up its environment.

A job that builds the Python binding takes uv from `astral-sh/setup-uv`, at
the version `[tool.uv] required-version` names, and the job's Python, in a
virtual environment that the later steps see. `uv sync --locked
--no-install-project` installs the `dev` group into it, exactly as
`uv.lock` pins it, and CMake's FindPython finds that interpreter first. The
Lint job and the coverage row take uv the same way, and install the `lint`
or the `coverage` group alone, with `--only-group`.

The branch rules require two checks: All checks, and SonarCloud's own report.
All checks runs whatever happened before it, because a job that a failure
skipped would otherwise count as passed, and it keeps the names of matrix
rows out of the rules. "Require branches to be up to date before merging" is
on, so two pull requests that pass apart but fail together cannot both merge
untested.

Each job keeps its compiler cache through `hendrikmuhs/ccache-action`, capped
at 500 MB and cleared of what the job did not use. The action installs its
own ccache binary on every runner, the version its pinned commit names, and
reports the cache's hit rate in the job summary. `main` saves the caches
every pull request starts from; a pull request saves its own, which only its
later pushes read; GitHub evicts the least recently used once the
repository's 10 GB are full. The fetched sources of the dependencies are
cached per operating system, keyed on every CMake file; a pull request whose
key is new starts from the newest such cache, and FetchContent downloads
again only what a changed declaration names. The packages vcpkg builds are
cached per runner image, compiler and triplet, keyed on its manifest; a new
key starts from the newest such cache, and vcpkg builds only what changed. Nothing else
manages the caches: an eviction makes a run slower, never wrong.

## Wheels

### The rexlib wheels

`ci.yml` builds them in the `wheels` jobs, one per platform, from the source
distribution the `sdist` job builds, so that every platform also checks that
the source distribution builds. cibuildwheel takes its
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

### The CUDA plugin's wheels

There is one distribution per CUDA major, `rexlib-cuda12` and
`rexlib-cuda13`, each with its `pyproject.toml` in `plugins/cuda/cuda12/` or
`cuda13/`, which builds the project one directory up
(`cmake.source-dir = ".."`). Both take their version from
`plugins/cuda/VERSION` and require `rexlib` of the same version,
and the root's extras `rexlib[cuda12]` and `rexlib[cuda13]` require them in
turn; the two extras exclude each other, since both plugins are named
`rexlib-cuda` and the second would be refused.

A wheel is Python-agnostic (`wheel.py-api = "py3"`) and manylinux only, for
Linux x86_64 and aarch64. Its package directory, `rexlib_cuda12/` or
`rexlib_cuda13/`, is the install prefix, so the plugin lands in its
`lib/rexlib-plugins/`, and the entry point `rexlib-cuda` in the group
`rexlib.plugins` names it for the default catalog. `CMAKE_INSTALL_RPATH`
reaches the CUDA runtime of NVIDIA's wheel three levels up, in
`nvidia/cuda_runtime/lib` for 12 and `nvidia/cu13/lib` for 13, and auditwheel
leaves both librexlib and libcudart out of the wheel.

cibuildwheel builds it once, with the newest CPython, in the manylinux image
after installing NVIDIA's compiler and runtime from its RHEL 8 repository,
without build isolation against the rexlib wheel of the same release, which
`REXLIB_WHEELHOUSE` names. It is tested by loading the plugin through the
default catalog, which needs no GPU. `ci.yml` does this in its `cuda-wheels`
jobs, with the rexlib wheels of the run.

## Releases

A release is a tag `v<version>` on `main`, where `<version>` is what
`VERSION` holds; before 1.0, `plugins/cuda/VERSION` holds the same. Both are
bumped in a pull request, and once it is merged whoever releases pushes the
tag.

The tag starts `.github/workflows/release.yml`. It calls `ci.yml` whole, so a
release is built and tested by the jobs every pull request runs, and then:

- checks the tag against `VERSION`, `VERSION` against `plugins/cuda/VERSION`,
  and every wheel and the source distribution against that version, and stops
  on any difference;
- publishes the `rexlib`, `rexlib-cuda12` and `rexlib-cuda13` wheels and the
  source distribution to PyPI by trusted publishing, from the environment
  `pypi`, with no token stored;
- creates the GitHub release, with notes GitHub generates from the pull
  requests merged since the last release and the distributions attached, which
  the ABI report compares against;
- deploys the documentation CI built to GitHub Pages, from the environment
  `github-pages`.

Run by hand (`workflow_dispatch`), it runs CI and checks the versions, and
publishes nothing: the dry run before a tag. GitHub offers it only for a
workflow on the default branch.
