# Modules

The submodules of the extension and the modules of the package, and what each
holds.

## The binding's modules

The submodules follow rexlib's directories, not its namespaces. rexlib has
four namespaces — `rexlib`, `ops`, `em` and `backends::cpu` — and none of
`hardware`, `dispatch`, `ndarray`, `numerical` or `functional` is one of
them: everything under those directories is flat in `namespace rexlib`,
`rexlib::device` and `rexlib::array` and `rexlib::add` alike. The paths are
the only place rexlib separates these areas, so the paths are what the
submodule names come from.

It reaches through nesting where rexlib's directories nest. An area under
`em/` sits inside `namespace em` the way `core/hardware/` sits inside
`namespace rexlib`, so it binds as `em.<area>` rather than as `em`, even where
its functions carry no qualifier of their own. The path is what keeps the
areas apart, which is why it is the path and not the function name that names
them. See #143.

`_binding` carries at its top level what the directories below it sit in:

| Module | From | Holds |
|---|---|---|
| `_binding` | `src/core/*.cpp` | `Version`, `Plugin`, `PluginManager`, `ServiceCatalog`, `rexlib_version`, `rexlib_binding_version` |
| `_binding.numerical` | `src/core/numerical/` | `NumericalType`, and the `float16_t` type caster |
| `_binding.ndarray` | `src/core/ndarray/` | `Array` and its buffer protocol, `ArrayDescriptor`, the functions that put an array's memory in a DLPack capsule and take one out, and the type caster that loads `const_array_ref` and `array_ref` from an `Array` |
| `_binding.hardware` | `src/core/hardware/` | Devices, sessions, queues, memory resources |
| `_binding.dispatch` | `src/core/dispatch/` | `ExecutionContext`, `Dispatcher`, `ProgramManager` |
| `_binding.functional` | `src/functional/` | The operations, each taking an explicit context |
| `_binding.em.functional` | `src/em/functional/` | The operations of `rexlib::em`, each taking an explicit context |

A submodule is created by the `main.cpp` above it, which then hands it to the
`bind_` function of the directory it stands for: `src/main.cpp` creates
`hardware` and hands it to `bind_hardware`, and a directory that nests does
the same one level down. Each names its submodules in the order the
declarations need.

## The package's modules

| Module | Adds |
|---|---|
| `_paths.py` | Locates the rexlib shipped inside the package; on Windows puts it on the DLL search path |
| `_catalog.py` | The process-wide default `ServiceCatalog` |
| `_session_pool.py` | One `DeviceSession` per device, process-wide |
| `_context.py` | The execution context active on the current thread |
| `_device.py` | `rexlib.device(...)`, the `with` block that activates one |
| `_functional.py` | The operations again, with `context` defaulting to the active one |
| `_ndarray.py` | Installs onto `Array` the Python operators, `__array__` and the two DLPack methods; holds `from_dlpack` |
| `em/` | The operations of `rexlib::em`, flat, with `context` defaulting to the active one |

`_paths` is imported first in `__init__.py`, and the order matters: on Windows
nothing else imports until the bundled library is findable. `_ndarray` is
imported for its side effect, before anything can hand out an `Array`.

DLPack is split between the two sides the way everything else is. The binding
builds a capsule and reads one, and reports the version of DLPack it was
compiled against. `_ndarray` holds what the protocol lets either side ask
for: the stream, the device, the version and the copy.

numpy is not imported by the package. `__array__` imports it when it is
called, and only numpy calls it.
