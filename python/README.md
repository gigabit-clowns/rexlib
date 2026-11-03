# Python binding for rexlib
This binding acts as an interface between the C++ interface of [rexlib](https://github.com/gigabit-clowns/rexlib) and the clients, written in Python.

## Install
To install this package, simply run:
```
pip install rexlib
```
This package wraps an installation of the rexlib C++ library and ships a copy of it inside the wheel. Any installation satisfying the version it asks for will do.

To build the binding from this repository, install what the build and the tests need (pip 25.1 or newer), then build it with the library and run its tests:
```
pip install --group dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DREXLIB_BUILD_PYTHON=ON \
  -DPython_EXECUTABLE=$(which python)
cmake --build build --parallel $(nproc)
ctest --test-dir build -L python
```
`pip install .` builds and installs a wheel instead. [docs/agents/building.md](docs/agents/building.md) has the details, including building the binding alone against an installed rexlib.

## Type stubs
Type stubs for the compiled extension are generated with [nanobind's stubgen](https://nanobind.readthedocs.io/en/latest/typing.html) when the package is installed, and end up in `rexlib/_binding/` next to the extension. They are not kept in the repository and there is nothing to run by hand: installing the package, from source or from a wheel, is enough.

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
