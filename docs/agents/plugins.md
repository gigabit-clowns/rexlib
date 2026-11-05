# Plugins

What a backend plugin exports, what it has to agree on with the core, how it
is loaded and shipped, which part of the core it may use, and how one is
written and admitted into the tree.

## What a plugin exports

A plugin is a shared object that expands `REXLIB_DEFINE_PLUGIN(type)`, from
`rexlib/core/plugin_definition.hpp`, once. It defines two functions with C
linkage: `rexlib_get_plugin_compatibility`, returning the
`plugin_compatibility` the plugin was built for, and `rexlib_get_plugin`,
returning the plugin, which it constructs on the first call. The loader
calls the first before anything else of the plugin; only the plugin's static
initializers run earlier, as the shared object loads. They must not call into
librexlib: a plugin the check would refuse could corrupt memory before it is
refused. `program_builder_registry` and `program_builder_registration` are
defined in their headers, so a plugin's builders can add themselves to a
registry of its own during static initialization, as the CPU's do, and the
plugin hands that registry to the `program_service` in `register_at`.

`plugin_compatibility` is plain data of a fixed layout, filled by the
`REXLIB_PLUGIN_COMPATIBILITY` macro where it is expanded: the version of
rexlib from `rexlib/core/version_macros.h`, and the C++ ABI from
`rexlib/core/platform/cpp_abi.hpp` (the Itanium or MSVC family, the standard
library and the setting that changes its ABI, the debug runtime of MSVC). A
macro rather than a function, because an inline function could resolve to the
core's copy and report the core's build. Fields are only appended; `size`
says how many the plugin knows.

## Loading

`plugin_loader` refuses a plugin with a message naming both sides when it
states no compatibility, or one the core cannot run with: before 1.0 another
minor version, from 1.0 another major version or a newer minor version, and
any other C++ ABI (`src/core/plugin_compatibility_check.cpp`). The family is
the ABI, not the compiler, so a core built with GCC loads a plugin built with
Clang. `plugin_manager` refuses a second plugin of a name it already holds,
which `discover_plugins` logs and skips. The shared object of a plugin it
accepts is never unloaded: once registered, what the plugin's code created
(backends, devices, queues, programs, builders) can be held anywhere, such as
a dispatcher, a Python pool or a user's variable, and outlive the manager, so
unloading it would leave them pointing at unmapped code. A shared object it
refuses is unloaded.

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

## Built-in plugins

The CPU backend is a plugin built into the library, and the core names no
backend. `cpu_plugin` registers the CPU device backend and the CPU's program
builders, which add themselves to `get_cpu_program_builder_registry()` through
`REXLIB_REGISTER_CPU_PROGRAM_BUILDER`. A built-in plugin adds itself to
`get_builtin_plugin_registry()` with a `builtin_plugin_registration`, and a
`service_catalog` constructed with `register_builtin_backends` registers
every built-in plugin at itself on construction, through the same
`register_at` as a loaded plugin.

## Plugin wheels

A plugin can also ship as a Python wheel. Its `pyproject.toml` makes a
package directory of its own, never `rexlib/`, the install prefix
(`wheel.install-dir`), so `rexlib_install_plugin` puts the plugin in that
prefix's `rexlib-plugins` directory as it does in any other, and the plugin's
CMake knows nothing of wheels; what only a wheel needs, such as an RPATH to a
library of another wheel, is a standard CMake variable the `pyproject.toml`
sets. The plugin's reference to librexlib resolves to the copy that
`import rexlib` already loaded, by SONAME on Linux, install name on macOS and
module name on Windows. The wheel declares an entry point in the group
`rexlib.plugins` whose value names that package.
`rexlib.discover_entry_point_plugins(manager)` (`python/rexlib/_plugins.py`)
loads every shared object directly in a `rexlib-plugins` directory of each
such package, and warns of and skips what it cannot load; the default catalog
does so when it is built, and a catalog the user builds loads only what it is
told. That is the one discovery route in Python; C++ has the search path,
`rexlib-plugins/` beside the library and `REXLIB_PLUGINS_PATH`.

## The sample plugin

`tests/install/plugin` is a plugin built as a third party builds one, against
the installed rexlib and with nothing but the installed CMake package. It adds
a device backend named `sample`, over host memory, with its own queue and
program type, and a program for `arange` into int32 that only that queue
accepts, so the install consumer, counting on that device, runs the plugin's
program or fails. Its `pyproject.toml` builds it as the wheel
`rexlib-sample`, with `rexlib_sample/` as its prefix and an entry point, and
`test_wheel.py` checks that the default catalog loads it. It is the template
for a new backend plugin.

## The backend surface

A plugin uses the part of the public headers made for it, and only a change
there touches plugins:

- the hardware layer, `include/rexlib/core/hardware/`: `device_backend`,
  `device`, `command_queue`, `memory_resource`, `memory_allocator`, `buffer`,
  `program` and the tokens and timelines of commands;
- `program_builder`, with `program_builder_registry` and
  `program_builder_registration`, and the operations and operand descriptors
  a builder reads;
- `plugin`, `REXLIB_DEFINE_PLUGIN` and `plugin_compatibility`;
- the service catalog: `service_catalog`, `device_service` and
  `program_service`;
- for programs that run on the CPU's device, `cpu_program` and
  `cpu_program_builder` of `include/rexlib/backends/cpu/`.

A change to it takes one of two forms, which its author chooses. Atomic: the
change, every plugin in the tree and the binding in one pull request, the
default for a small change. Expand and contract: one pull request adds the new
form beside the old, each plugin moves to it in a pull request of its own, and
a last one removes the old form, `main` valid at every step. An operation is
one pull request: its declaration, its CPU program, its functional API, its
binding and their tests; another backend adds its program when it is ready.

## Writing a plugin

`tests/install/plugin` is the template, and each step below is a file of it:

1. A CMake project of its own that finds rexlib with `find_package(rexlib)`
   and builds a `MODULE` linked to `rexlib::rexlib`, with hidden visibility,
   so that only the two entry points are exported (`CMakeLists.txt`).
2. A class derived from `rexlib::plugin` with a name, a version and a
   `register_at` that registers its device backend at the catalog's
   `device_service` and hands its builders to the `program_service`
   (`sample_plugin.cpp`).
3. `REXLIB_DEFINE_PLUGIN` of that class, once, in one source
   (`plugin_hook.cpp`).
4. A backend of its own, if it runs on a device of its own: a
   `device_backend`, a `device`, a `command_queue` and a program type the
   queue runs (`sample_device_backend.cpp` and the files beside it).
5. Its program builders, each adding itself to a `program_builder_registry`
   the plugin owns through a `program_builder_registration`, and nothing else
   at namespace scope that calls into librexlib
   (`sample_arange_program_builder.cpp`).
6. `rexlib_install_plugin(<target>)`, which installs it where rexlib looks
   for plugins, beside the library, with the RPATH back to it.
7. Built with the same C++ ABI as the rexlib it runs with, which the loader
   checks.

As a wheel, its `pyproject.toml` makes a package directory of its own the
install prefix, is tagged `py3` when it has no extension module, requires
`rexlib`, and declares its entry point in the group `rexlib.plugins`
(`pyproject.toml`). Its repair leaves librexlib out, and an RPATH to the
library of another wheel, such as NVIDIA's runtime for the CUDA plugin, is
`CMAKE_INSTALL_RPATH` in the `pyproject.toml`.

## Admission into the tree

A plugin lives in this repository if and only if hosted CI builds it on every
pull request, as a required check; its tests that need no device run there;
and at least two maintainers own it in `CODEOWNERS`. The bar is the same for
every plugin in the tree. What differs between backends, what CI can check of
them and how many operations they implement, is stated in their
documentation, not in CI. A backend that misses the bar lives in a repository
of its own and builds against the published rexlib; since a plugin is already
a project of its own, moving it in or out is a directory move and its CI
jobs.
