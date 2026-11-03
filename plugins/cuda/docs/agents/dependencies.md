# Dependencies

How each dependency is fetched or found, and how it is pinned.

Every dependency but rexlib and the CUDA Toolkit is fetched and built with
the plugin by default, static and position independent: Boost (Container,
Intrusive and Unordered, linked as `Boost::headers`) and spdlog, and for the
tests Catch2 and trompeloeil. Turning `REXLIB_CUDA_BUILD_DEPENDENCIES` off
finds them all installed instead, with `find_package`. Turning off
`REXLIB_CUDA_BUILD_<DEP>` (`BOOST`, `SPDLOG`, `CATCH2`, `TROMPELOEIL`) does
that for one of them. `cmake_dependent_option` declares those toggles, so
that the global one forces the others off.

A dependency is a branch where it is used, in `CMakeLists.txt` for the
plugin and in `tests/CMakeLists.txt` for the tests. Built, it is a call to
`rexlib_cuda_fetch_<dep>()` with what pins the release it fetches. Found, it
is the `find_package` call, with the oldest version that works. Each
`rexlib_cuda_fetch_<dep>()` has a module of its own in `cmake/modules/`,
reads nothing but its arguments, and sets the options of the project it
fetches in its own scope. A new dependency gets a toggle, a branch, a module
and a manager in `renovate.json`.

Those calls hold the only pins, in forms Renovate keeps up to date. What is
released on GitHub is pinned by the commit of its release tag,
`COMMIT <sha>) # <tag>`, as the actions of the workflows are. Boost is
pinned by its tag and the SHA256 of the archive it attaches to the release,
since the archive of a commit lacks its libraries, which are submodules.
Renovate bumps the versions, not the hashes, so a bump of Boost fails to
configure until the hash CMake reports is pasted in.

The `cuda-toolkit` requirement in `pyproject.toml` is not a version this
project follows: it is a placeholder `select_cuda_major.py` rewrites for each
wheel, and Renovate leaves it alone.
