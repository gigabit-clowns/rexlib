# Wheels, pull requests and continuous integration

How the wheels are built, how a pull request is opened, and what each
workflow does.

## Wheels

One repository builds one distribution per CUDA major: `rexlib-cuda11`,
`rexlib-cuda12` and `rexlib-cuda13`. `select_cuda_major.py` rewrites the
distribution name and the `cuda-toolkit` requirement before each build, line by
line so that the comments in `pyproject.toml` survive.

A wheel is Python-agnostic (`wheel.py-api = "py3"`) and manylinux only. It
installs into the `rexlib` package directory (`wheel.install-dir = "rexlib"`),
because rexlib reports its plugin directory relative to its own prefix. The
plugin's RPATH reaches the CUDA runtime of NVIDIA's own wheels, three levels
up in site-packages, through `REXLIB_CUDA_RUNTIME_RPATH`, and auditwheel is
told to leave both librexlib and libcudart out of the wheel.

The wheels build against a rexlib built from `external/rexlib` beforehand, in
the same container, not against an installed `rexlib` wheel.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

| Workflow | Does |
|---|---|
| `build-and-test.yml` | Builds and tests with CMake, then the SonarQube scan |
| `deploy.yml` | Builds the wheels for each CUDA major on x86_64 and aarch64, and publishes them |
| `release.yml` | Tags and releases, through the shared workflow of the organisation |
| `clean-up-caches.yml`, `keep-caches-warm.yml` | Manage the Actions cache |

`build-and-test.yml` covers Ubuntu with gcc and clang on CUDA 12, Ubuntu 22.04
with CUDA 11 and Windows with CUDA 13, in Debug. It builds rexlib from the
submodule into a prefix first, which is most of the wall clock and what the
compiler cache is for. The cache and the fetched sources are saved only on
`main`. The compilation database of the gcc build is what the scan reads.

Publishing to PyPI is wired up but disabled. Every push to `main` updates the
`development` pre-release with its wheels.
