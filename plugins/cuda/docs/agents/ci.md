# Wheels, pull requests and continuous integration

How the wheels are built, how a pull request is opened, and what each
workflow does.

## Wheels

There is one distribution per CUDA major, `rexlib-cuda12` and
`rexlib-cuda13`, each with its `pyproject.toml` in `cuda12/` or `cuda13/`,
which builds the project one directory up (`cmake.source-dir = ".."`). Both
take their version from `VERSION` and require `rexlib` of the same version,
and the root's extras `rexlib[cuda12]` and `rexlib[cuda13]` require them in
turn; the two extras exclude each other, since both plugins are named
`rexlib-cuda` and the second would be refused.

A wheel is Python-agnostic (`wheel.py-api = "py3"`) and manylinux only, for
Linux x86_64 and aarch64. Its package directory, `rexlib_cuda12/` or
`rexlib_cuda13/`, is the install prefix, so the plugin lands in its
`lib/rexlib-plugins/`, and the entry point `rexlib-cuda` in the group
`rexlib.plugins` names it for the default catalog. `CMAKE_INSTALL_RPATH`
reaches the CUDA runtime of NVIDIA's wheel three levels up, in
`nvidia/cuda_runtime/lib` for 12 and `nvidia/cu13/lib` for 13, and auditwheel
leaves both librexlib and libcudart out of the wheel.

cibuildwheel builds it once, with the newest CPython, in the manylinux image
after installing NVIDIA's compiler and runtime from its RHEL 8 repository,
without build isolation against the rexlib wheel of the same release, which
`REXLIB_WHEELHOUSE` names. It is tested by loading the plugin through the
default catalog, which needs no GPU. The library's workflow does this in its
`cuda-wheels` jobs, with the rexlib wheels of the run.

## Pull requests

Whoever opens a pull request assigns it to themselves and gives it the label,
among the repository's own, that matches what it changes: `bug` for a fix,
`enhancement` for something new, `documentation` when only documentation
changes, and so on.

## Continuous integration

The plugin is built and tested by the library's workflow, one job per CUDA
major; see [the library's CI](../../../../docs/agents/ci.md). Each builds
the plugin against the library that the library's Linux job of the same
architecture installed, as a third party builds against an installed rexlib.
