# Conventions

How the code is written: formatting, files, names, documentation, errors,
logging and the plugin boundary.

Indentation is tabs. Lines stay within 80 columns.

Declarations go in a `.hpp`, definitions that must be visible go in an `.inl`
the header includes at its end, and everything else goes in a `.cpp`. Every
source starts with `// SPDX-License-Identifier: GPL-3.0-only`, and every
CMake file with `# SPDX-License-Identifier: GPL-3.0-only`.

A class that implements one of rexlib's interfaces is named
`<qualifier>_<interface>`, as in rexlib, the qualifier saying which one it is:
`cuda_device`, `cuda_command_queue`, `cuda_device_backend` and `cuda_buffer`
implement `device`, `command_queue`, `device_backend` and `buffer`, while
`pinned_memory_resource`, `caching_memory_allocator` and `stream_timeline` are
named after the memory or the mechanism behind them. Its files carry the same
name. The name stands on its own: `namespace rexlib::cuda`
keeps the plugin's symbols apart from those of other backends, and is never
what tells an implementation from its interface, so inside it a plain `device`
is rexlib's.

Declarations are documented with Doxygen comments: `@brief`, `@param`,
`@return` and `@throws`.

Every call into the CUDA runtime goes through one of three macros from
`src/hardware/error.hpp`, which record the call and where it was made:

| Macro | On failure |
|---|---|
| `REXLIB_CUDA_CHECK` | Throws `cuda::error` |
| `REXLIB_CUDA_CHECK_ALLOCATION` | Throws `std::bad_alloc` when memory ran out, `cuda::error` otherwise |
| `REXLIB_CUDA_CHECK_NO_THROW` | Writes to the standard error stream; for destructors and other `noexcept` code |

Logging goes through `REXLIB_CUDA_LOG_TRACE` … `REXLIB_CUDA_LOG_CRITICAL`, and
`REXLIB_CUDA_LOG_LEVEL` sets the level compiled in.

The plugin and the core it is loaded into have to agree on compiler, standard
library and, on Windows, the runtime: the plugin boundary is a C++ ABI
boundary. See rexlib's AGENTS.md.
