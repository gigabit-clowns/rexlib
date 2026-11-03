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
run the preset commands a developer runs, so a compiler warning fails them
as it fails a contributor's build. They use public actions only, pinned by
digest, on runner images named by version rather than `-latest`, and check
the repository out without keeping its credentials. Every job has a timeout,
and a newer push to a pull request cancels the run it supersedes; runs on
`main` are never cancelled.

| Job | What it checks |
|---|---|
| One per platform and toolchain | Linux x86_64 with gcc and with clang, Linux aarch64 with gcc, macOS arm64, Windows x86_64 and arm64 with MSVC, Windows x86_64 with MinGW. Each builds the `debug` preset and runs every suite, the header check and the install test |
| Address and undefined behaviour sanitizers; thread sanitizer | The `sanitize` and `sanitize-thread` presets, built with clang |
| CMake 3.18, installed dependencies | The one job without presets: CMake 3.18.0, Release, the suites, and a check that no dependency reaches rexlib's install. vcpkg installs every dependency into a prefix at the baseline `.github/vcpkg/vcpkg.json` names, and the `find_package` branch of each finds it, as a packager's would |
| Documentation | The `docs` preset; a Doxygen warning fails it |
| Lint | The `lint` preset: every file git tracks against `.editorconfig` |
| All checks | Fails unless every other job succeeded |

The rows hold the oldest gcc and clang of the supported Ubuntu LTS images; the
sanitizer jobs build with the newest clang.

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
at 500 MB and cleared of what the job did not use. `main` saves the caches
every pull request starts from; a pull request saves its own, which only its
later pushes read; GitHub evicts the least recently used once the
repository's 10 GB are full. The fetched sources of the dependencies are
cached per operating system, keyed on every CMake file; a pull request whose
key is new starts from the newest such cache, and FetchContent downloads
again only what a changed declaration names. The packages vcpkg builds are
cached keyed on its manifest. Nothing else
manages the caches: an eviction makes a run slower, never wrong.

No workflow publishes releases yet.
