# The CUDA plugin of rexlib

The CUDA hardware backend of [rexlib](../../README.md), a high-performance
Cryo-EM and Cryo-ET image processing library. It is a plugin: a shared object
that rexlib's plugin manager loads at run time, which registers a CUDA
`device_backend`, built against an installed rexlib as any third-party plugin
is.

From Python, install it as an extra of rexlib, for CUDA 12 or 13, on Linux
x86_64 or aarch64:

```
pip install "rexlib[cuda12]"
```

From C++, install the library, then build the plugin against it from the root
of the repository:

```
cmake -S . -B build/rexlib -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build/rexlib --parallel
cmake --install build/rexlib --prefix build/rexlib-install
cmake -S plugins/cuda -B build/cuda -Drexlib_ROOT=build/rexlib-install
cmake --build build/cuda --parallel
cmake --install build/cuda --prefix build/rexlib-install
```

Its conventions are in [AGENTS.md](../../AGENTS.md) and
[docs/agents/](../../docs/agents/).
