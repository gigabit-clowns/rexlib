# Layout

Where each part of the plugin lives.

| Path | Holds |
|---|---|
| `src/cuda_plugin.hpp`, `src/cuda_plugin.cpp` | `cuda_plugin`, the `plugin` the core sees: its name, version and what it registers |
| `src/plugin_hook.cpp` | `rexlib_get_plugin`, the one exported symbol |
| `src/hardware/` | The implementation of rexlib's hardware interface: backend, device, queue, event, memory resources, errors |
| `src/hardware/caching_allocator/` | The caching memory allocator, modelled on PyTorch's |
| `src/config.hpp` | Every tunable of the backend, as a macro the build may override |
| `src/logger.hpp` | The `REXLIB_CUDA_LOG_*` macros over spdlog |
| `tests/unitary/`, `tests/integration/` | Catch2 suites, with trompeloeil for mocks |
| `cmake/modules/` | One `rexlib_cuda_fetch_*.cmake` per dependency, `rexlib_cuda_architectures` and `rexlib_cuda_enable_warnings` |
| `scripts/select_cuda_major.py` | Rewrites `pyproject.toml` for one CUDA major before a wheel is built |
| `external/rexlib` | The rexlib this plugin is developed against, a submodule |

Everything is in `namespace rexlib::cuda`.
