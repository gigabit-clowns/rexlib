# Wheels, pull requests and continuous integration

How the wheels are built, how a pull request is opened, and what each
workflow does.

## Wheels

No workflow builds wheels yet. These are the decisions the wheels of the
plugin's own repository were built on, which the wheels of this one keep:
one distribution per CUDA major, `rexlib-cuda12` and `rexlib-cuda13`.

A wheel is Python-agnostic (`wheel.py-api = "py3"`) and manylinux only. It
installs into the `rexlib` package directory (`wheel.install-dir = "rexlib"`),
because rexlib reports its plugin directory relative to its own prefix. The
plugin's RPATH reaches the CUDA runtime of NVIDIA's own wheels, three levels
up in site-packages, through `REXLIB_CUDA_RUNTIME_RPATH`, and auditwheel is
told to leave both librexlib and libcudart out of the wheel.

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
