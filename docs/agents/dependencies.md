# Dependencies

How the library, the binding and the CUDA plugin fetch, pin or find what they
depend on.

## The library

Every dependency is fetched and built with rexlib by default, static, and
ends up inside its library. Turning `REXLIB_USE_SYSTEM_DEPENDENCIES` on
finds them all installed instead, with `find_package`, as a distribution
builds rexlib. `.github/vcpkg/vcpkg.json` installs every one of them at the
baseline it names: `vcpkg install --x-manifest-root=.github/vcpkg
--x-install-root=<dir>`, then configure with
`-DREXLIB_USE_SYSTEM_DEPENDENCIES=ON -DCMAKE_PREFIX_PATH=<dir>/<triplet>`.
Turning on `REXLIB_USE_SYSTEM_<DEP>` (`BOOST`, `SPDLOG`, `HALF`,
`POCKETFFT`, `EIGEN`, and for the tests `CATCH2` and `TROMPELOEIL`) does that
for one of them. `cmake_dependent_option` declares those toggles, so that the
global one forces the others on. The names follow CMake's own
`CMAKE_USE_SYSTEM_LIBRARY_<NAME>`, and leave `REXLIB_BUILD_*` to the parts
of rexlib itself.

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

## The binding

nanobind comes from pip and is never fetched: it is in `build-system.requires`
for pip, and in the `dev` group for a build by CMake itself. CMake asks the
interpreter it found where nanobind's package is, `python -m nanobind
--cmake_dir`, unless `nanobind_ROOT` or `nanobind_DIR` says so.

DLPack is fetched by default, through `rexlib_python_fetch_dlpack()` in
`cmake/modules/`, pinned as `COMMIT <sha>) # <tag>` in `python/CMakeLists.txt`.
It is one header, and only the header is taken: its own build file is not
read. The library's `REXLIB_USE_SYSTEM_DEPENDENCIES`, or
`REXLIB_USE_SYSTEM_DLPACK` alone, on finds an installed one instead; no
version is asked of it, since its package reports 0.6 whatever the release.

What the Python tests and the development tools need is in the dependency
groups of `pyproject.toml`, which `uv.lock` pins; see [tests](tests.md).

## The CUDA plugin

Every dependency but rexlib and the CUDA Toolkit is fetched and built with
the plugin by default, static and position independent: Boost (Container,
Intrusive and Unordered, linked as `Boost::headers`) and spdlog, and for the
tests Catch2 and trompeloeil. Turning `REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES`
on finds them all installed instead, with `find_package`. Turning on
`REXLIB_CUDA_USE_SYSTEM_<DEP>` (`BOOST`, `SPDLOG`, `CATCH2`, `TROMPELOEIL`)
does that for one of them. `cmake_dependent_option` declares those toggles,
so that the global one forces the others on.

A dependency is a branch where it is used, in `plugins/cuda/CMakeLists.txt`
for the plugin and in `plugins/cuda/tests/CMakeLists.txt` for the tests, as
in the library: built, a call to `rexlib_cuda_fetch_<dep>()` with what pins
the release it fetches, each with a module of its own in
`plugins/cuda/cmake/modules/`; found, the `find_package` call, with the oldest
version that works. The pins take the forms of the library's: the commit of a
release tag, `COMMIT <sha>) # <tag>`, and for Boost its tag and the SHA256 of
the archive it attaches to the release. `renovate.json` has managers for the
library's pins alone, not for these nor for the binding's DLPack.

Each wheel's `pyproject.toml` requires `cuda-toolkit[cudart]` of its CUDA
major, `==12.*` or `==13.*`, which NVIDIA's runtime wheels satisfy; the
toolkit its build installs is pinned to a minor version, 12.9 or 13.4, in its
`before-all`.
