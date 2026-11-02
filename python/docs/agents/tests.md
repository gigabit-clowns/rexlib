# Tests

How the tests run, where a test goes, and what they may depend on.

They are Python tests against the installed package. There are no C++ tests:
the binding has no logic of its own worth testing from C++, and what it does
have is only observable from Python.

`dlpack_buffer` is tested the same way, through `rexlib.from_dlpack`.
`tests/ndarray/test_dlpack.py` builds a tensor by hand with `ctypes` for it:
one whose releases can be counted, and which can be what a library does not
export, such as a tensor on another device or of another version.

```
./scripts/run-tests.sh
./scripts/run-tests.sh --coverage
```

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

The package has to be installed with `REXLIB_PYTHON_BUILD_TESTING=ON`, which
is what builds the dummy plugins in `tests/assets/`. The runner sets pytest's
rootdir to the repository and, with `--coverage`, writes `coverage.xml`
against `conf/coverage.ini`, which maps `python/rexlib` onto the installed
copy the tests actually import.

`tests/` mirrors the binding's module structure. A test goes in the directory
named after the module whose surface it exercises, not the module it happens
to call through: `tests/dispatch/test_active_context.py` reaches `rexlib.zeros`
to observe context resolution, and context resolution is what it is about.
Tests for what `_binding` exposes at its top level stay at the root.

| Directory | Covers |
|---|---|
| `tests/` | `Version`, `PluginManager`, `ServiceCatalog` |
| `tests/numerical/` | `NumericalType` |
| `tests/ndarray/` | `ArrayDescriptor`, the operators installed onto `Array`, the buffer protocol and DLPack |
| `tests/hardware/` | Devices, sessions, memory resources, the session pool |
| `tests/dispatch/` | `ExecutionContext`, the active context, `rexlib.device(...)` |
| `tests/functional/` | The operations |

There are no `__init__.py` files and no `conftest.py`. Test module names are
therefore unique across the whole tree, and a fixture belongs to the file that
uses it — several files define a `__setup_context` of their own rather than
sharing one. Keep new fixtures file-local until something actually needs to
share them.

`tests/CMakeLists.txt` descends into `assets/` and nothing else: the Python
files are collected by pytest from where they are, so moving one needs no
change to CMake.

numpy is a test requirement, pinned per Python version from 2.2 to 2.5, all of
which take and give versioned DLPack tensors. What the binding itself does
with a versioned or a read-only tensor is tested with the hand-made one.

torch and JAX are not test requirements. A test that needs one starts with
`pytest.importorskip`, so it runs where the library is installed and is
skipped in CI.
