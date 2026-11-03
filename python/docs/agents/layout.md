# Layout

Where things are in the repository, and which side of the binding a change
belongs to.

| Path | Holds |
|---|---|
| `src/` | The nanobind binding, 38 `.cpp` and 39 `.hpp`, compiled into the `rexlib._binding` extension |
| `python/rexlib/` | The Python package, 10 `.py`: everything the binding cannot express |
| `tests/` | pytest suites, mirroring the binding's module structure |
| `tests/assets/` | Two dummy plugins, built by CMake, that the plugin tests discover |
| `scripts/` | Development tools: the test runner, the stub generator and collector, the wheel check |
| `cmake/modules/` | `rexlib_python_stubs.cmake`, which installs the type stubs, `rexlib_python_enable_warnings.cmake` and `rexlib_python_fetch_dlpack.cmake` |
| `conf/` | `coverage.ini` |
| `external/rexlib` | The rexlib the repository is developed against, a submodule |

There are two sides, and which one a change belongs to is the first question
to ask.

`src/` mirrors rexlib's own directory structure, and the module it produces
mirrors the C++ API: same names, same argument order, same required arguments.
It does not decide anything. `python/rexlib/` is where the API becomes Python:
default arguments, implicit context, operators, process-wide state. Its
modules carry a leading underscore and `__init__.py` re-exports what they
hold, so that the public surface is one flat namespace rather than a set of
import paths.

A public subpackage is the exception to that, and `em/` is the one: its module
path *is* the API, so the directories are named without an underscore and each
carries an `__all__` of its own. Anything private inside one still takes the
underscore.

`em` holds the operations of `rexlib::em`, which rexlib declares under
`em/functional/` the way it declares its own under `functional/`. They are
flat in `rexlib.em`, as the operations of `rexlib` are flat in `rexlib`, each
with its context defaulted by `em/_functional.py`. Image I/O, once an area of
`em`, now lives in [vitrio](https://github.com/gigabit-clowns/vitrio), a
package of its own that takes and gives arrays through DLPack and the buffer
protocol.

The rule that follows: if a change could be expressed in either place, it goes
in `python/rexlib/`. C++ is the expensive side to change and the one that has
to keep matching rexlib.

