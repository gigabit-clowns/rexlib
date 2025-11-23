# Python binding for the core of Xmipp4
This binding acts as an interface between the C++ interface functions of [Xmipp4's core](https://github.com/gigabit-clowns/xmipp4-core) and the clients, written in Python.

## Install
To install this package, simply run:
```
pip install xmipp4-python
```
To install in development mode, you will first need to install `xmipp4-core` in your current environment, and then, from the root of this project, run:
```
CMAKE_BUILD_PARALLEL_LEVEL=$(nproc) pip install . --no-build-isolation -v -Ccmake.define.BUILD_TESTING=ON
```
To run the tests for this project (only avaiable when installed in development mode), run:
```
./scripts/run-tests.sh
```

## SonarCloud status
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)

### Ratings
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=sqale_index)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)

### Specific metrics
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=bugs)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=duplicated_lines_density)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_xmipp4-python&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_xmipp4-python)
