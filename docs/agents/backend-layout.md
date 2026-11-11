# Backend layout

How the sources of a backend are arranged: the roles a file can play, the
order they depend in, and where a new program, skeleton, piece of math or
library adaptor goes. The CPU backend, `src/backends/cpu/`, is the instance
described here; the CUDA plugin, `plugins/cuda/src/`, follows the same
scheme as its programs arrive.

## Roles

A file sits in the directory of its role. A role includes only the roles
above it in this table. `config.hpp` is a leaf that any role may include,
and the plugin class at the root is the bottom, composing everything.

| Role | Holds | On CUDA |
|---|---|---|
| `numerical/` | Element semantics in the compute type: loading, storing and casting elements, `boolean_cast` | Device-side counterparts |
| `hardware/` | The implementation of the hardware interface: device, queue, thread pool, `functor_program` | `hardware/` |
| `dispatch/` | The backend's `program_builder` base, its registry and the registration macro | The same |
| `loops/` | What sees only layouts, pointers and a schedule: loop drivers, scheduling, strides (`strides/`), element indices (`indices/`), layout preparation (`layouts/`) | Launch geometry, grid-stride loops |
| `math/` | Standalone mathematical objects, one directory per subject | `__device__` math |
| `skeletons/` | Program templates parameterized by a kernel, which see an operand descriptor or an operation. The root holds their shared base; each skeleton has a directory | The same |
| `<library>/<algorithm>/` | The adaptor over one algorithm of one algorithm library, named `<library>_<algorithm>` | `cufft/fft/`, `cublas/gemm/` |
| `programs/<category>/` | The program of one operation, in the catalog's category | The same |
| `em/` | `math/`, `skeletons/` and `programs/` again, for EM code | The same |

## The CPU backend

```
src/backends/cpu/
  config.hpp  cpu_plugin.hpp  cpu_plugin.cpp
  numerical/
  hardware/
  dispatch/
  loops/        strides/  indices/  layouts/
  math/         scalar/  special/  random/  windows/  fourier/  geometry/
  skeletons/    elementwise/  reduction/  batch_reduce_map/
                batched_core/  fourier/  roll/
  eigen/gemm/
  pocketfft/fft/
  programs/     arithmetic/  assignment/  bitwise/  compare/  complex/
                creation/  fourier/  linalg/  logical/  numeric/  power/
                random/  reduction/  rounding/  selection/  trigonometric/
  em/
    math/       optics/
    skeletons/  slice_insertion/
    programs/   preprocessing/standardize_background/
                reconstruction/insert_slices/
                reconstruction/insert_weighted_slices/
                reconstruction/insert_ctf_slices/
```

The CUDA plugin has `config.hpp`, `cuda_plugin`, `logger.hpp` and
`plugin_hook.cpp` at its root, and `hardware/` below it.

## Rules

1. A file goes where its nature puts it, never where its user is. `math/`
   holds objects that stand on their own; a skeleton's directory holds what
   only makes sense inside its iteration: its plan, functor, tuning, tiling,
   chunks and row views.
2. A file that sees only layouts, pointers and a schedule belongs to
   `loops/`. One that sees an `operand_descriptor` or an operation belongs to
   a skeleton.
3. A skeleton or program is EM if it includes `<rexlib/em/...>`, and its
   internals go with it under `em/`. Standalone math is EM only if it
   includes `<rexlib/em/...>` itself. Nothing outside `em/` includes
   `<rexlib/em/...>`.
4. Skeletons, adaptors and programs may include catalog headers
   (`<rexlib/ops/...>`, `<rexlib/em/...>`). Math includes only their
   parameter types. `numerical/`, `hardware/`, `dispatch/` and `loops/`
   include none.
5. Libraries come in two kinds.
   - An algorithm library computes something for the backend: Eigen,
     pocketfft, MKL, pdqsort; cuFFT and cuBLAS on CUDA. It gets an adaptor
     directory per algorithm. An adaptor may include a skeleton's plan; a
     skeleton never includes an adaptor. A skeleton takes the algorithm as a
     policy, and the program passes the adaptor in.
   - A programming library is a way to write code: Highway; cub on CUDA. Any
     role may use it, as it uses the standard library. `highway/` holds
     shared glue only, and a Highway kernel goes where rules 1 and 2 put it.
6. Sibling skeletons may include each other's plans and functors, as long as
   the includes form no cycle.
7. A program is `programs/<category>/<operation>_program.cpp`. A program of
   more than one file gets a directory, `programs/<category>/<operation>/`,
   per element type translation units included. The catalog category decides
   the directory, never the skeleton the program uses.
8. No directory or file is named after "operation" or "op", the noun
   "dispatcher", or "routine". A role reusing a core directory name
   (`numerical/`, `hardware/`, `dispatch/`) is that concept's backend side.
9. Tests and benchmarks mirror the tree. A test may include any role.

## Adding

- **A program.** One `<operation>_program.cpp` in the operation's category
  under `programs/`, or under `em/programs/` for an EM operation. It
  registers a skeleton instantiated with its kernel factory, through the
  skeleton's `REXLIB_REGISTER_*_PROGRAM_BUILDER` macro. Its kernels and
  weights go beside it, in a directory of its own.
- **A skeleton.** A directory under `skeletons/`, or `em/skeletons/`, holding
  its program builder and everything its iteration needs. It derives from
  `typed_program_builder` and takes its kernel factory, or its algorithm, as
  a template parameter.
- **Math.** A file in the subject's directory under `math/`, or a new
  subject.
- **An algorithm library adaptor.** `<library>/<algorithm>/`, named
  `<library>_<algorithm>`, with the support predicate of the element types
  the library computes in. A skeleton that takes the algorithm as a policy,
  as `skeletons/fourier/` takes `pocketfft_fft`, defines what the policy
  provides. The library's headers are included only by the adaptor's own
  sources, never by a header another role includes.

## Shared between backends

`math/random/` must draw the same bits on every backend (see
[random numbers](random-numbers.md)), and `rule_type_dispatch`,
`type_dispatch_support_query`, `operand_data_types`, `core_input_indices`
and `default_kernel_factory` in `skeletons/` do not depend on the backend.
They live in the CPU backend until a second backend implements programs.
Then they move to an installed location, which changes the plugin surface
(see [plugins](plugins.md)).
