# Dependencies

How rexlib fetches, pins or finds what it depends on.

Every dependency is fetched and built with rexlib by default, static, and
ends up inside its library. Turning `REXLIB_BUILD_DEPENDENCIES` off finds
them all installed instead, with `find_package`, as a distribution builds
rexlib. `.github/vcpkg/vcpkg.json` installs every one of them at the
baseline it names: `vcpkg install --x-manifest-root=.github/vcpkg
--x-install-root=<dir>`, then configure with `-DREXLIB_BUILD_DEPENDENCIES=OFF
-DCMAKE_PREFIX_PATH=<dir>/<triplet>`. Turning off `REXLIB_BUILD_<DEP>` (`BOOST`, `SPDLOG`, `HALF`,
`POCKETFFT`, `EIGEN`, and for the tests `CATCH2` and `TROMPELOEIL`) does that
for one of them. `cmake_dependent_option` declares those toggles, so that the
global one forces the others off.

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
