# Dependencies

How the library, the binding and the CUDA plugin get, pin or find what they
depend on.

## The sources in third_party

The sources of every dependency are in the repository, in `third_party/` for
the library and the binding and in `plugins/cuda/third_party/` for the
plugin, which carries its own so that its directory stays self contained. Each
is a git submodule, at the commit of a release tag, which `.gitmodules` names
as the submodule's `branch`. pocketfft, which makes no releases, follows its
`cpp` branch. half, a single header with no repository, is a copy of its
release in `third_party/half/`.

Boost is one submodule per library it needs, at the same release, in
`third_party/boost/libs/`: the libraries asked for and the ones they need in
turn, 36 for the library and 14 for the plugin, `headers` included. Boost's
CMake is the submodule `tools/cmake`, and `third_party/boost/CMakeLists.txt` is
the root file of Boost's superproject, which reads the version from
Boost.Config. A Boost release that makes a library need another stops the
configure, naming the missing `Boost::` target, until its submodule is added.

A clone needs the submodules: `git clone --recurse-submodules`, or `git
submodule update --init` in an existing clone. They are shallow
(`shallow = true`), so only the pinned commit of each is fetched. The source
distribution carries them, without their tests, documentation and examples,
so it builds with nothing downloaded.

## The library

Every dependency is built with rexlib by default, static, from its sources in
`third_party/`, and ends up inside its library. Turning
`REXLIB_USE_SYSTEM_DEPENDENCIES` on finds them all installed instead, with
`find_package`, as a distribution builds rexlib. `vcpkg.json` installs every
one of them at the baseline it names: `vcpkg install --x-install-root=<dir>`
from the root, then configure with
`-DREXLIB_USE_SYSTEM_DEPENDENCIES=ON -DCMAKE_PREFIX_PATH=<dir>/<triplet>`.
Turning on `REXLIB_USE_SYSTEM_<DEP>` (`BOOST`, `SPDLOG`, `HALF`,
`POCKETFFT`, `EIGEN`, and for the tests `CATCH2` and `TROMPELOEIL`) does that
for one of them. `cmake_dependent_option` declares those toggles, so that the
global one forces the others on. The names follow CMake's own
`CMAKE_USE_SYSTEM_LIBRARY_<NAME>`, and leave `REXLIB_BUILD_*` to the parts
of rexlib itself.

A dependency is a branch where it is used, in `CMakeLists.txt` for the
library and in `tests/CMakeLists.txt` for the tests. Built, it is a call to
`rexlib_add_<dep>()` with the directory of its sources. Found, it is the
`find_package` call, with the oldest version that works. Each
`rexlib_add_<dep>()` has a module of its own in `cmake/modules/`, reads
nothing but its arguments, sets the options of the dependency in its own
scope, and adds it through `rexlib_add_dependency()`. That declares it to
FetchContent with its `SOURCE_DIR`, and stops the configure, naming the
command to run, when the submodule is not checked out. Through FetchContent,
a project that embeds rexlib keeps control: the first declaration of a name
wins, and `FETCHCONTENT_SOURCE_DIR_<NAME>` points one at another checkout. A
new dependency gets a toggle, a branch, a module and a submodule.

half and pocketfft ship no CMake package config anywhere, so their system path
goes through this project's own `Findhalf.cmake` and `Findpocketfft.cmake`.
Neither header carries a version, so neither module can check one.

Every dependency is private and none appears in a public header, so the
installed package asks only for `Threads` whichever way they were obtained,
and an archive of a default build is self contained.

## The binding

nanobind comes from pip and is not in `third_party/`: it is in
`build-system.requires` for pip, and in the `build` group, which `dev`
includes, for a build by CMake itself. CMake asks the interpreter it found
where nanobind's package is, `python -m nanobind --cmake_dir`, unless
`nanobind_ROOT` or `nanobind_DIR` says so.

DLPack is built from `third_party/dlpack` by default, through
`rexlib_python_add_dlpack()` in `cmake/modules/`. It is one header, and only
the header is taken: its own build file is not read. The library's
`REXLIB_USE_SYSTEM_DEPENDENCIES`, or `REXLIB_USE_SYSTEM_DLPACK` alone, on finds
an installed one instead; no version is asked of it, since its package reports
0.6 whatever the release.

What the Python tests and the development tools need is in the dependency
groups of `pyproject.toml`; see [the tools from PyPI](#the-tools-from-pypi).

## The CUDA plugin

Every dependency but rexlib and the CUDA Toolkit is built with the plugin by
default, static and position independent, from `plugins/cuda/third_party/`:
Boost (Container, Intrusive and Unordered, linked as `Boost::headers`) and
spdlog, and for the tests Catch2 and trompeloeil. Turning
`REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES` on finds them all installed instead,
with `find_package`. Turning on `REXLIB_CUDA_USE_SYSTEM_<DEP>` (`BOOST`,
`SPDLOG`, `CATCH2`, `TROMPELOEIL`) does that for one of them.
`cmake_dependent_option` declares those toggles, so that the global one forces
the others on. Inside the library's build (`REXLIB_BUILD_CUDA_PLUGIN`), the
library turns the plugin's global switch on, as its own, and both find every
dependency installed.

A dependency is a branch where it is used, in `plugins/cuda/CMakeLists.txt`
for the plugin and in `plugins/cuda/tests/CMakeLists.txt` for the tests, as
in the library: built, a call to `rexlib_cuda_add_<dep>()` with the directory
of its sources, each with a module of its own in
`plugins/cuda/cmake/modules/` and added through `rexlib_cuda_add_dependency()`;
found, the `find_package` call, with the oldest version that works.

Each wheel's `pyproject.toml` requires `cuda-toolkit[cudart]` of its CUDA
major, `==12.*` or `==13.*`, which NVIDIA's runtime wheels satisfy; the
toolkit its build installs is pinned to a minor version, 12.9 or 13.4, in its
`before-all`.

## The tools from PyPI

What the builds, the tests and the checks take from PyPI is in the dependency
groups of `pyproject.toml`, which `uv.lock` pins with the hash of each file
(see [tests](tests.md)):

- `build`: scikit-build-core, CMake, Ninja and nanobind, what a build without
  build isolation, or by CMake itself, needs;
- `test`: numpy, pytest and pytest-cov;
- `dev`: `build` and `test`, what `uv sync` installs;
- `lint`: clang-tidy, editorconfig-checker and ruff, what the `lint` preset
  runs;
- `coverage`: gcovr.

editorconfig-checker's package is a source distribution that downloads the
program from its GitHub release when it is built, so `uv.lock` holds the hash
of the package, not of the program. CMake 3.18.0, the oldest the projects
support, is in no group: the jobs that build with it install it themselves.

## Updates

Renovate keeps the pins up to date through `renovate.json`, each update
14 days after its release:

- the actions of the workflows, with `actions.lock` regenerated in the same
  pull request by the `gh-actions-lock` version of `constraints`;
- the dependency groups and build requirements of every `pyproject.toml`,
  with `uv.lock`, and once a month the packages `uv.lock` pins beyond them;
- the submodules of `third_party/` and `plugins/cuda/third_party/`, to the
  newest release tag, every Boost library in one pull request, and a
  dependency the library and the plugin share in both at once; pocketfft to
  the newest commit of its `cpp` branch;
- the baseline of `vcpkg.json`, to vcpkg's newest commit, once a month;
- uv's `required-version`, and the `gh-actions-lock` version of the Lint job
  and of `constraints`, in one pull request.

Renovate also opens pull requests for the vulnerabilities that GitHub's
Dependabot alerts report. By hand: half's copy, the CUDA toolkit minors of
the CUDA jobs and wheels, CMake 3.18.0, the runner images, and the Python
versions of `requires-python` and of the workflows.
