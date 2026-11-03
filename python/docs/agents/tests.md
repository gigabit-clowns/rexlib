# Tests

How the tests run, where a test goes, and what they may depend on.

They are Python tests against the installed package. There are no C++ tests:
the binding has no logic of its own worth testing from C++, and what it does
have is only observable from Python.

`dlpack_buffer` is tested the same way, through `rexlib.from_dlpack`.
`tests/python/ndarray/test_dlpack.py` builds a tensor by hand with `ctypes`
for it: one whose releases can be counted, and which can be what a library
does not export, such as a tensor on another device or of another version.

```
ctest --test-dir build -L python
```

In a build with the binding, `rexlib-python-install` installs the build into
a directory of the build tree, and `rexlib-python` runs pytest on
`tests/python/` against that install. `PYTEST_ADDOPTS="--cov
--cov-report=xml"` adds coverage, which `[tool.coverage]` in
`pyproject.toml` maps from the installed copy the tests import onto
`python/rexlib`.

What the tests need is the `test` group of `pyproject.toml` (`pip install
--group test`, which `dev` includes).

`uv.lock` pins every package of the groups, those they pull in included, for
every Python and platform, with the hash of each file. CI installs the
`test` group from it, `uv export --frozen --only-group test` given to
`pip install --require-hashes --only-binary :all:`, so a file that changed
or a source distribution of a dependency is refused. Only the uv of
`[tool.uv] required-version` writes it: after a change to a group, run
`uv lock`. Renovate regenerates it with every bump of a pin of
`pyproject.toml`, refreshes what the pins pull in through its lock file
maintenance, and bumps `required-version`.

The dummy plugins in `tests/python/assets/` are built into a directory of the
build tree and never installed; the CTest test names that directory to
pytest in `REXLIB_PYTHON_TEST_PLUGINS_DIR`.

`tests/python/` mirrors the binding's module structure. A test goes in the
directory named after the module whose surface it exercises, not the module
it happens to call through: `tests/python/dispatch/test_active_context.py`
reaches `rexlib.zeros` to observe context resolution, and context resolution
is what it is about.
Tests for what `_binding` exposes at its top level stay at the root.

| Directory | Covers |
|---|---|
| `tests/python/` | `Version`, `PluginManager`, `ServiceCatalog`, the type stubs |
| `tests/python/numerical/` | `NumericalType` |
| `tests/python/ndarray/` | `ArrayDescriptor`, the operators installed onto `Array`, the buffer protocol and DLPack |
| `tests/python/hardware/` | Devices, sessions, memory resources, the session pool |
| `tests/python/dispatch/` | `ExecutionContext`, the active context, `rexlib.device(...)` |
| `tests/python/ops/fourier/` | `FourierNormalization` |
| `tests/python/em/functional/` | The operations of `rexlib.em` |
| `tests/python/em/ops/parameters/` | `GriddingProfile`, `GriddingWindow`, `CtfOptics` |
| `tests/python/functional/` | The operations |

There are no `__init__.py` files and no `conftest.py`. Test module names are
therefore unique across the whole tree, and a fixture belongs to the file that
uses it — several files define a `__setup_context` of their own rather than
sharing one. Keep new fixtures file-local until something actually needs to
share them.

`tests/python/CMakeLists.txt` descends into `assets/` and registers the two
CTest tests: the Python files are collected by pytest from where they are,
so moving one needs no change to CMake.

numpy is a test requirement, pinned per Python version from 2.2 to 2.5, all of
which take and give versioned DLPack tensors. What the binding itself does
with a versioned or a read-only tensor is tested with the hand-made one.

torch and JAX are not test requirements. A test that needs one starts with
`pytest.importorskip`, so it runs where the library is installed and is
skipped in CI.
