# rexlib
Core library for the xmipp4 framework

[![Build and Test](https://github.com/gigabit-clowns/rexlib/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/gigabit-clowns/rexlib/actions/workflows/build-and-test.yml)
[![Run tests with memcheck](https://github.com/gigabit-clowns/rexlib/actions/workflows/test-with-memcheck.yml/badge.svg)](https://github.com/gigabit-clowns/rexlib/actions/workflows/test-with-memcheck.yml)

## SonarCloud status
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)

### Ratings
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=sqale_index)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)

### Specific metrics
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=bugs)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=duplicated_lines_density)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)

## Platform support
On Windows, the published binaries (wheels and conda packages) are built
with MSVC. This is a hard requirement, not just the default: the C++ ABI
(symbol mangling, vtable layout) is not compatible across compilers on
Windows, so any code that links against rexlib there must also be built 
with MSVC. Linking against these binaries with MinGW/gcc will fail with 
undefined-reference errors on exported C++ symbols (e.g. class vtables).

This only affects linking on Windows. rexlib itself is still built
and tested with gcc (via MinGW) in CI, since that only involves compiling
rexlib from source with a single, consistent toolchain — the
incompatibility only shows up when mixing toolchains across the binary
boundary of a prebuilt artifact.
