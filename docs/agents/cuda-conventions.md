# CUDA conventions

What the code of the CUDA plugin, under `plugins/cuda/`, follows beyond
[code-style](code-style.md) and [naming](naming.md): C++20, required rather
than preferred, and everything in `namespace rexlib::cuda`.

A class that implements one of rexlib's interfaces is named
`<qualifier>_<interface>`, as in rexlib, the qualifier saying which one it is:
`cuda_device`, `cuda_command_queue`, `cuda_device_backend` and `cuda_buffer`
implement `device`, `command_queue`, `device_backend` and `buffer`, while
`pinned_memory_resource`, `caching_memory_allocator` and `stream_timeline` are
named after the memory or the mechanism behind them. Its files carry the same
name. The name stands on its own: `namespace rexlib::cuda` keeps the plugin's
symbols apart from those of other backends, and is never what tells an
implementation from its interface, so inside it a plain `device` is rexlib's.

Declarations are documented with Doxygen comments: `@brief`, `@param`,
`@return` and `@throws`.

Every call into the CUDA runtime goes through one of three macros from
`plugins/cuda/src/hardware/error.hpp`, which record the call and where it was
made:

| Macro | On failure |
|---|---|
| `REXLIB_CUDA_CHECK` | Throws `cuda::error` |
| `REXLIB_CUDA_CHECK_ALLOCATION` | Throws `std::bad_alloc` when memory ran out, `cuda::error` otherwise |
| `REXLIB_CUDA_CHECK_NO_THROW` | Writes to the standard error stream; for destructors and other `noexcept` code |

Logging goes through `REXLIB_CUDA_LOG_TRACE` … `REXLIB_CUDA_LOG_CRITICAL`, and
`REXLIB_CUDA_LOG_LEVEL` sets the level compiled in.

The plugin and the core it is loaded into have to agree on the C++ ABI: the
plugin boundary is a C++ ABI boundary, which the loader checks; see
[plugins](plugins.md).
