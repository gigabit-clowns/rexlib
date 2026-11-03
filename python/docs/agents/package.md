# The package

The conventions of the Python code under `python/rexlib/`.

Indentation is tabs, on both sides. Lines stay within 80 columns. Every source
file, the CMake files included, starts with
`SPDX-License-Identifier: GPL-3.0-only`.

`from __future__ import annotations` goes at the top of every module that
annotates anything, so that no annotation is evaluated at import. That is what
lets one name something imported only under `TYPE_CHECKING`, such as a type
of a library the package does not depend on at run time. Python 3.14 defers
annotations by itself; until 3.13 is dropped, the import is what does it.

Module-level state uses two leading underscores — `__pool`, `__default_catalog`,
`__local`. Python mangles nothing at module scope, so this is a convention
rather than a mechanism, and it is the one that marks the difference between
the module's own state and something a caller might reasonably reach for.

Public functions carry a docstring with `Args:` and `Returns:`; ruff enforces
their presence under pydocstyle, in pep257 convention.

`__init__.py` re-exports with `as`, `from ._x import y as y`, which is what
makes the name public to a type checker, and lists everything again in
`__all__`. Both are needed and both are checked.

Operations live at the top of `rexlib`, not in a submodule, matching the C++
side where they are in `rexlib` itself and the array libraries they are used
alongside. `dispatch` and `hardware` stay submodules: they are reached for
when setting up, not when computing.

ruff is the authority and runs in CI as `ruff check .`. Its configuration is
`ruff.toml`; `D206` is disabled because the project indents with tabs.
