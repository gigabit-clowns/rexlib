# Python binding for rexlib
This binding acts as an interface between the C++ interface of [rexlib](https://github.com/gigabit-clowns/rexlib) and the clients, written in Python.

## Install
To install this package, simply run:
```
pip install rexlib
```
This package wraps an installation of the rexlib C++ library and ships a copy of it inside the wheel. Any installation satisfying the version it asks for will do.

The rexlib this repository is developed against is the `external/rexlib` submodule, so clone with it:
```
git clone --recurse-submodules https://github.com/gigabit-clowns/rexlib-python.git
```
If you have no rexlib installed, build the submodule into a prefix:
```
cmake -S external/rexlib -B build/rexlib -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=~/.local -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_TESTING=OFF
cmake --build build/rexlib --target install --parallel $(nproc)
```
Then, to install in development mode from the root of this project:
```
CMAKE_PREFIX_PATH=~/.local CMAKE_BUILD_PARALLEL_LEVEL=$(nproc) \
  pip install . --no-build-isolation -v -Ccmake.define.REXLIB_PYTHON_BUILD_TESTING=ON
```
To run the tests for this project (only avaiable when installed in development mode), run:
```
./scripts/run-tests.sh
```

## Type stubs
Type stubs for the compiled extension are generated with [pybind11-stubgen](https://github.com/sizmailov/pybind11-stubgen) while the bindings are built, and end up in `rexlib/_binding/` next to the extension. They are not kept in the repository and there is nothing to run by hand: installing the package, from source or from a wheel, is enough.

Generating them means importing the extension, which a build cannot do when it is compiling for another architecture. Those builds are handed stubs made elsewhere instead, through `REXLIB_STUBS_DIR`:
```
pip install . -C cmake.define.REXLIB_STUBS_DIR=<directory holding the .pyi files>
```
The path may be relative to the project. Stubs describe the Python API, so the same ones are correct for every platform. A build that can neither generate nor be given them fails rather than producing an untyped package.

## SonarCloud status
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)

### Ratings
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=sqale_index)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)

### Specific metrics
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=bugs)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=duplicated_lines_density)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib-python&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib-python)
