# Tests

How the suites of the library, the binding and the CUDA plugin run, where a
test goes, and what the tests may depend on. How a test stands in for an
interface is in [test-doubles](test-doubles.md); the install test is in
[building](building.md).

## The library

```
ctest --test-dir build
```

`tests/unitary/` and `tests/integration/` are Catch2 suites. Each suite
carries its name as a CTest label, so `ctest -L unitary` and
`ctest -L integration` run one of them. The label is set whichever way the
cases were registered. Under CMake 3.18 each suite is a single test, since
listing the cases needs the JSON support of CMake 3.19.

`REXLIB_REGISTER_TESTS_PER_BINARY` registers one test per executable rather
than one per case, because a memory checker pays its start-up on every test
CTest runs. Discovering each case makes it re-analyse the whole binary once per
case, which took the memcheck job past six hours.

On Windows the library and the test executables are built into one
directory, `bin/` of the build tree, unless `CMAKE_RUNTIME_OUTPUT_DIRECTORY`
is given: Windows finds a library beside the executable that loads it, and
a plugin a test loads finds `rexlib.dll` there too.

## The binding

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
`python/rexlib`. A build with `REXLIB_ENABLE_COVERAGE` adds `--cov` itself.

What the tests need is the `test` group of `pyproject.toml` (`pip install
--group test`, which `dev` includes, or `uv sync`). `uv.lock` pins every
package of the groups, those they pull in included, for every Python and
platform, with the hash of each file; CI installs from it with
`uv sync --locked`, which refuses a lockfile that no longer matches
`pyproject.toml`. Only the uv of `[tool.uv] required-version` writes it:
after a change to a group, run `uv lock`.

The dummy plugins in `tests/python/assets/` are built into a directory of the
build tree and never installed; the CTest test names that directory to
pytest in `REXLIB_PYTHON_TEST_PLUGINS_DIR`. A test that loads them is marked
`dummy_plugins`, so that every wheel, which never carries them, runs the rest
of the suite as cibuildwheel's test.

`tests/python/` mirrors the binding's module structure. A test goes in the
directory named after the module whose surface it exercises, not the module
it happens to call through: `tests/python/dispatch/test_active_context.py`
reaches `rexlib.zeros` to observe context resolution, and context resolution
is what it is about.
Tests for what `_binding` exposes at its top level stay at the root.

| Directory | Covers |
|---|---|
| `tests/python/` | `Version`, `PluginManager`, `ServiceCatalog`, the entry-point plugins, the type stubs |
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

numpy is a test requirement, pinned per Python version, and every version
pinned takes and gives versioned DLPack tensors. What the binding itself does
with a versioned or a read-only tensor is tested with the hand-made one.

torch and JAX are not test requirements. A test that needs one starts with
`pytest.importorskip`, so it runs where the library is installed and is
skipped in CI.

## The CUDA plugin

```
ctest --test-dir build/cuda --output-on-failure
```

The unitary suite embeds the plugin's objects and exercises its internals,
mostly the caching allocator against mocks. The integration suite loads the
built plugin through rexlib's plugin manager, which is why the plugin is
copied beside the test executable. The labels and the registration are as
the library's. A test that needs a GPU skips itself when no CUDA capable
device is present, which is always the case in CI.

On Windows the rexlib DLL is copied beside each test executable, since nothing
else puts it on the loader's search path.
