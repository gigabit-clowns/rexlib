# Pull requests and continuous integration

How a pull request is opened and what the workflows check.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

`.github/workflows/ci.yml` runs on every pull request and every push to
`main`, with no path filter: a change runs every job it could break. The jobs
run the preset commands a developer runs. They use public actions only, pinned by
digest, on runner images named by version rather than `-latest`, and check
the repository out without keeping its credentials. Every job has a timeout,
and a newer push to a pull request cancels the run it supersedes; runs on
`main` are never cancelled.

| Job | What it checks |
|---|---|
| One per platform and toolchain | Linux x86_64 with gcc and with clang, Linux aarch64 with gcc, macOS arm64, Windows x86_64 and arm64 with MSVC, Windows x86_64 with MinGW. Each builds the `debug` preset with warnings as errors and runs every suite, the header check and the install test |
| Address and undefined behaviour sanitizers; thread sanitizer | The `sanitize` and `sanitize-thread` presets, built with clang. These are the default build: the dependencies are fetched and built with rexlib, and warnings do not fail it |
| CMake 3.18 | The one job without presets: CMake 3.18.0, Release with warnings as errors, and the suites |
| Documentation | The `docs` preset; a Doxygen warning fails it |
| Lint | The `lint` preset: clang-tidy, pinned, on the library's sources as they compile, and every file git tracks against `.editorconfig`. The dependencies are installed, as in the jobs with warnings as errors |
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
a leak of any kind, or any other error it reports, fail the test. Then gcovr
measures the coverage, and the SonarQube scan reads it with the compilation
database of the same build.

A row is named by platform and toolchain, never by compiler version, which
is the runner image's to choose. A `run` step reads the runner's values from
the environment (`$RUNNER_TEMP`); an action's `with`, which no shell expands,
reads them from the context (`${{ runner.temp }}`).

MSVC builds through Ninja, which is what lets ccache cache it, with its debug
information embedded in each object for the same reason;
`ilammy/msvc-dev-cmd` sets up its environment.

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

No workflow publishes releases yet.
