# Plugins

What a backend plugin exports and what it has to agree on with the core.

A plugin is a shared object exporting `rexlib_get_plugin`, returning a
`const plugin*`. `plugin_loader` resolves that symbol by name, so the contract
is the symbol and the `plugin` interface, not the C++ ABI of the loader.

It is still a C++ ABI boundary for everything the plugin touches: the plugin
and the core have to agree on compiler, standard library, and on Windows the
runtime and whether it is the debug one.

`dynamic_library` takes paths in UTF-8 on every platform and resolves every
symbol a library needs as it loads (`RTLD_NOW | RTLD_LOCAL`; Windows binds a
DLL's imports at load), so a plugin that needs a symbol the running rexlib
lacks fails with `dynamic_library_error` instead of at its first call. On
Windows it loads with `LoadLibraryExW` and
`LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS`, so the
DLLs a plugin needs are found beside it, and neither `PATH` nor the working
directory is searched. The platform calls are in
`src/core/system/dynamic_library_handle_{posix,windows}.cpp`, which CMake
picks between.

The CPU backend is a plugin built into the library, and the core names no
backend. `cpu_plugin` registers the CPU device backend and the CPU's program
builders, which add themselves to `get_cpu_program_builder_registry()` through
`REXLIB_REGISTER_CPU_PROGRAM_BUILDER`. A built-in plugin adds itself to
`get_builtin_plugin_registry()` with a `builtin_plugin_registration`, and a
`service_catalog` constructed with `register_builtin_backends` registers
every built-in plugin at itself on construction, through the same
`register_at` as a loaded plugin.
