# Backend layout

How the sources of a backend are arranged: the roles a file can play, the
order they depend in, and where a new program, skeleton, helper or library
adaptor goes. The CPU backend, `src/backends/cpu/`, is the instance
described here; the CUDA plugin, `plugins/cuda/src/`, follows the same
scheme as its programs arrive.

## Roles

The roles are a closed set. A file sits in the directory of its role, and a
role includes only the roles above it in this table. `config.hpp` is a leaf
that any role may include, and the plugin class at the root is the bottom,
composing everything.

| Role | Holds | On CUDA |
|---|---|---|
| `numerical/` | Element semantics: how a single value of the backend's element types behaves. Loading, storing and casting, truthiness, division rounding, the ordering of unordered values. Constants and special functions come from Boost.Math | Device-side counterparts |
| `random/` | The random key convention: the Philox generator, key derivation, draws | The same, drawing the same bits |
| `hardware/` | The implementation of the hardware interface: device, queue, thread pool, `functor_program` | `hardware/` |
| `dispatch/` | The backend's `program_builder` base, its registry and the registration macro | The same |
| `loops/` | What sees only layouts, pointers and a schedule: loop drivers, scheduling, strides (`strides/`), element indices (`indices/`), layout preparation (`layouts/`) | Launch geometry, grid-stride loops |
| `parameters/<type>/` | The evaluation of a catalog parameter type, mirroring `ops/parameters/` | `__device__` evaluators |
| `skeletons/` | Program templates parameterized by a kernel, which see an operand descriptor or an operation. The root holds their shared base; each skeleton has a directory | The same |
| `libraries/<library>/<algorithm>/` | The adaptor over one algorithm of one algorithm library, named `<library>_<algorithm>` | `libraries/cufft/fft/`, `libraries/cublas/gemm/` |
| `programs/<category>/` | The program of one operation, in the catalog's category | The same |
| `em/` | `parameters/`, `skeletons/` and `programs/` again, for EM code | The same |

Only `parameters/` and `programs/` grow with the catalog, `skeletons/` with
the iteration patterns, and `libraries/` with the algorithm libraries.
Nothing else adds a directory to the root.

## Where a helper goes

A helper is code that a program or a skeleton uses but that is neither of
them. It has exactly one of five homes:

1. It defines how a single value behaves: `numerical/`. A constant or a
   special function is Boost.Math's, not a helper.
2. It draws or derives random bits: `random/`.
3. It evaluates a catalog parameter type: `parameters/<type>/`, as
   `em/parameters/ctf_optics/` evaluates `em::ctf_optics`.
4. Its meaning comes from one operation's definition: that program's
   directory, as `centred_ball` defines the background of
   `standardize_background`.
5. It belongs to one iteration (its plan, tuning, tiling, culling geometry,
   chunks, row views or phase conventions): that skeleton's directory.

A helper that fits none of them is a design question, not a new directory.
When a second operation needs a helper that sits in a program or a skeleton,
the need it shares names a catalog concept, and the helper moves with that
concept: to `parameters/` once the catalog has the parameter type, or to a
skeleton both programs use.

## The CPU backend

```
src/backends/cpu/
  config.hpp  cpu_plugin.hpp  cpu_plugin.cpp
  numerical/
  random/
  hardware/
  dispatch/
  loops/          strides/  indices/  layouts/
  skeletons/      elementwise/  reduction/  batch_reduce_map/
                  batched_core/  fourier/  roll/
  libraries/      eigen/gemm/  pocketfft/fft/
  programs/       arithmetic/  assignment/  bitwise/  compare/  complex/
                  creation/  fourier/  linalg/  logical/  numeric/  power/
                  random/  reduction/  rounding/  selection/  trigonometric/
  em/
    parameters/   ctf_optics/  gridding_window/
    skeletons/    slice_insertion/
    programs/     optics/
                  preprocessing/standardize_background/
                  reconstruction/insert_slices/
                  reconstruction/insert_weighted_slices/
                  reconstruction/insert_ctf_slices/
```

The CUDA plugin has `config.hpp`, `cuda_plugin`, `logger.hpp` and
`plugin_hook.cpp` at its root, and `hardware/` below it.

## Rules

1. A file goes where its nature puts it, never where its user is. Its nature
   is what defines it: a value's semantics, the random convention, a
   parameter type, an operation or an iteration.
2. A file that sees only layouts, pointers and a schedule belongs to
   `loops/`. One that sees an `operand_descriptor` or an operation belongs to
   a skeleton.
3. What decides which elements an iteration visits goes with the iteration.
   An object built while planning and evaluated in the loop is one object,
   and stays in one place.
4. No directory is named after a discipline (`math/`, `physics/`, `utils/`,
   `common/`), nor after "operation" or "op", the noun "dispatcher", or
   "routine". A role reusing a framework name (`numerical/`, `random/`,
   `hardware/`, `dispatch/`, `parameters/`) is that concept's backend side.
5. Code is EM if it includes `<rexlib/em/...>`, and its internals go with it
   under `em/`. Nothing outside `em/` includes `<rexlib/em/...>` or a file
   under `em/`.
6. Skeletons, adaptors and programs may include catalog headers
   (`<rexlib/ops/...>`, `<rexlib/em/...>`). A parameter evaluator includes
   its parameter type. `numerical/`, `random/`, `hardware/`, `dispatch/` and
   `loops/` include none.
7. Libraries come in two kinds.
   - An algorithm library computes something for the backend: Eigen,
     pocketfft, MKL, pdqsort; cuFFT and cuBLAS on CUDA. It gets an adaptor
     directory per algorithm under `libraries/`. An adaptor may include a
     skeleton's plan; a skeleton never includes an adaptor. A skeleton takes
     the algorithm as a policy, and the program passes the adaptor in.
   - A programming library is a way to write code: Highway; cub on CUDA. Any
     role may use it, as it uses the standard library. A Highway kernel goes
     where the rules above put the kernel.
8. Sibling skeletons may include each other's plans and functors, as long as
   the includes form no cycle. A program includes no other program's files.
9. A program is `programs/<category>/<operation>_program.cpp`. A program of
   more than one file gets a directory, `programs/<category>/<operation>/`,
   per element type translation units included. The catalog category decides
   the directory, never the skeleton the program uses.
10. Tests and benchmarks mirror the tree. A test may include any role.

## Adding

- **A program.** One `<operation>_program.cpp` in the operation's category
  under `programs/`, or under `em/programs/` for an EM operation. It
  registers a skeleton instantiated with its kernel factory, through the
  skeleton's `REXLIB_REGISTER_*_PROGRAM_BUILDER` macro. Its kernels, weights
  and helpers go beside it, in a directory of its own.
- **A skeleton.** A directory under `skeletons/`, or `em/skeletons/`, holding
  its program builder and everything its iteration needs. It derives from
  `typed_program_builder` and takes its kernel factory, or its algorithm, as
  a template parameter.
- **A helper.** The home that [where a helper goes](#where-a-helper-goes)
  gives it.
- **An algorithm library adaptor.** `libraries/<library>/<algorithm>/`,
  named `<library>_<algorithm>`, with the support predicate of the element
  types the library computes in. A skeleton that takes the algorithm as a
  policy, as `skeletons/fourier/` takes `pocketfft_fft`, defines what the
  policy provides. The library's headers are included only by the adaptor's
  own sources, never by a header another role includes.

## Shared between backends

`random/` must draw the same bits on every backend (see
[random numbers](random-numbers.md)), and `rule_type_dispatch`,
`type_dispatch_support_query`, `operand_data_types`, `core_input_indices`,
`own_core_input_indices`, `index_array` and `default_kernel_factory` in
`skeletons/` do not depend on the backend.
They live in the CPU backend until a second backend implements programs.
Then they move to an installed location, which changes the plugin surface
(see [plugins](plugins.md)).
