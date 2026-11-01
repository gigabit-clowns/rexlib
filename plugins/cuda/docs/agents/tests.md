# Tests

How the test suites run and what each covers.

```
ctest --test-dir build/cuda --output-on-failure
```

The unitary suite embeds the plugin's objects and exercises its internals,
mostly the caching allocator against mocks. The integration suite loads the
built plugin through rexlib's plugin manager, which is why the plugin is
copied beside the test executable. Each suite carries its name as a CTest
label, so `ctest -L unitary` and `ctest -L integration` run one of them.
Under CMake 3.18 each suite is a single test, since listing the cases needs
the JSON support of CMake 3.19. A test that needs a GPU skips itself when
no CUDA capable device is present, which is always the case in CI.

On Windows the rexlib DLL is copied beside each test executable, since nothing
else puts it on the loader's search path.
