# Dependencies

How the library, the binding and the CUDA plugin ask for, pin and get what
they depend on.

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

A dependency is a `find_package` call where it is used, in `CMakeLists.txt`
for the library and in `tests/CMakeLists.txt` for the tests, with the oldest
version that works, and nothing else: where it comes from is the business of
whoever configures. Configured with nothing more, rexlib finds every
dependency installed, as a distribution builds it, with CMake 3.18 or newer.
`.github/vcpkg/vcpkg.json` installs every one of them at the baseline it
names, as CI does: `vcpkg install --x-manifest-root=.github/vcpkg
--x-install-root=<dir>`, then configure with
`-DCMAKE_PREFIX_PATH=<dir>/<triplet>`.

`cmake/rexlib_dependency_provider.cmake` is rexlib's dependency provider.
Given to the first `project()` through `CMAKE_PROJECT_TOP_LEVEL_INCLUDES`, as
the presets and `pyproject.toml` do, it answers every `find_package` of a
dependency it knows with that dependency built from `third_party/`, static,
added once, at the first call, and leaves any other to search as usual. It
needs CMake 3.24. With the presets, `-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=`
turns it off, and every dependency is found installed again.

Its table names, for each package, the module that adds it and its directory
in `third_party/`. Each `rexlib_add_<dep>()` has a module of its own in
`cmake/modules/`, reads nothing but its arguments, sets the options of the
dependency in its own scope, and adds it through `rexlib_add_dependency()`.
That declares it to FetchContent with its `SOURCE_DIR`, stops the configure,
naming the command to run, when the submodule is not checked out, and keeps
the version the dependency's own build states. A call stops the configure
when the version it asks for is newer than that one, or when a component it
requires has no `<package>::<component>` target. Boost is built from every
library checked out in `third_party/boost/libs/`. A new dependency gets a
`find_package` call, a module, a row in the provider and a submodule.

half and pocketfft ship no CMake package config anywhere, so their installed
path goes through this project's own `Findhalf.cmake` and
`Findpocketfft.cmake`. Neither header carries a version, so neither module
can check one.

Every dependency is private and none appears in a public header, so the
installed package asks only for `Threads` whichever way they were obtained,
and an archive of a build with the provider is self contained.

## The binding

nanobind comes from pip and is not in `third_party/`: it is in
`build-system.requires` for pip, and in the `build` group, which `dev`
includes, for a build by CMake itself. CMake asks the interpreter it found
where nanobind's package is, `python -m nanobind --cmake_dir`, unless
`nanobind_ROOT` or `nanobind_DIR` says so.

DLPack is a `find_package(dlpack)` in `python/CMakeLists.txt`, which rexlib's
provider answers from `third_party/dlpack`, through
`rexlib_python_add_dlpack()`. It is one header, and only the header is taken:
its own build file is not read. No version is asked of it, since its package
reports 0.6 whatever the release.

What the Python tests and the development tools need is in the dependency
groups of `pyproject.toml`; see [the tools from PyPI](#the-tools-from-pypi).

## The CUDA plugin

The plugin asks for its dependencies the same way: `find_package` calls in
`plugins/cuda/CMakeLists.txt` and `plugins/cuda/tests/CMakeLists.txt`, for
Boost (Container, Intrusive and Unordered, linked as `Boost::headers`) and
spdlog, and for the tests Catch2 and trompeloeil. Its own provider,
`plugins/cuda/cmake/rexlib_cuda_dependency_provider.cmake`, which its presets
and the wheels' `pyproject.toml` inject, builds them from
`plugins/cuda/third_party/`, static and position independent, through the
`rexlib_cuda_add_<dep>()` modules of `plugins/cuda/cmake/modules/`. rexlib
itself is never provided: it is found installed.

Inside the library's build (`REXLIB_BUILD_CUDA_PLUGIN`), the plugin's
`find_package` calls go to the library's provider, so one configure holds one
copy of each dependency, built from the root's `third_party/`. That carries
the Boost libraries the plugin needs too, and builds spdlog position
independent. Without a provider, both find the installed ones.

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
of the package, not of the program. CMake 3.18.0 and 3.24.0, the oldest the
library and the plugin support, are in no group: the jobs that build with
them install them themselves.

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
- the baseline of `.github/vcpkg/vcpkg.json`, to vcpkg's newest commit, once
  a month;
- uv's `required-version`, and the `gh-actions-lock` version of the Lint job
  and of `constraints`, in one pull request.

Renovate also opens pull requests for the vulnerabilities that GitHub's
Dependabot alerts report. By hand: half's copy, the CUDA toolkit minors of
the CUDA jobs and wheels, CMake 3.18.0 and 3.24.0, the runner images, and the
Python versions of `requires-python` and of the workflows.
