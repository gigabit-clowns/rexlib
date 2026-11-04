# The Python binding of rexlib

The nanobind binding of [rexlib](../README.md), a high-performance Cryo-EM and
Cryo-ET image processing library. `src/` is compiled into the extension
`rexlib._binding`, and `rexlib/` is the package that adds what Python expects
on top of it.

It is built with the library, from the root of the repository:

```
pip install --group dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DREXLIB_BUILD_PYTHON=ON \
  -DPython_EXECUTABLE=$(which python)
cmake --build build --parallel
ctest --test-dir build -L python
```

`pip install .`, from the root, builds and installs the `rexlib` wheel
instead. The type stubs of the extension are generated when the package is
installed, from a source build or a wheel; there is nothing to run by hand.

The binding's conventions are in [AGENTS.md](../AGENTS.md) and
[docs/agents/](../docs/agents/), its open questions in
[DECISIONS.md](DECISIONS.md), and its settled work in
[BACKLOG.md](BACKLOG.md).
