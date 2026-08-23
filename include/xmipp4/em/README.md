# em — CryoEM domain component (placeholder)

This directory reserves the layout for the CryoEM-specific operation set.
It follows the same conventions as the generic components and must be kept
in sync with them.

## Layout

```
include/xmipp4/em/
├── ops/<category>/     CryoEM operation definitions
│                       (e.g. ops/projection/project_3d_operation.hpp)
└── functional/         user-facing verbs (e.g. project_3d(...))
src/em/
├── ops/<category>/     operation implementations
└── functional/         verb implementations
```

## Namespace

Everything in this component lives in `xmipp4::em` (one namespace per
component). Operations and verbs share it: `em::project_3d_operation`,
`em::project_3d(...)`.

## Dependencies

`em` may depend on `core`, `ops` and `functional`. Nothing outside `em/`
may include `xmipp4/em/` headers, and `core` must never learn about this
component. Keep the arrows pointing downwards.

## Backend builders

Every backend implements its `program_builder`s for em operations under
`builders/em/<category>/`, mirroring the catalog:

- in-tree CPU: `src/backends/cpu/builders/em/<category>/`
- plugin repositories (e.g. rexlib-cuda) mirror the same skeleton and
  expose their builder SDK as `<xmipp4/backends/<name>/...>`.

The **category** decides the directory; the builder family does not. A
generic family template serves operations from several categories at
once, so its instantiations are scattered across the category
directories, and a category directory holds whichever families its
operations happen to need. Among the generic builders `vecdot` is a
reduction living in `linalg/`, `fftshift` a roll living in `fourier/`,
and `arange` a sequence living in `creation/`. None of those is a
misfiling.

The family templates themselves, and the adapters to their kernel
concepts, sit at the top level of `builders/`. What a family plans lives
in `plans/`, beside `builders/`, because a plan is stored in the program
and read on every execution rather than discarded after `build()`.
Arithmetic shared by more than one operation lives in `kernels/`.

Each builder is one self-contained `.cpp` ending in an
`REX_REGISTER_*_PROGRAM_BUILDER` macro, which registers it at static
initialization; there is no registrar list to add to.

## Coverage policy

The in-tree CPU builders are the reference tier: every em operation must
have a (possibly naive) CPU builder before it is merged. Accelerator
backends are best effort; optimized plugins may override the reference
builders by reporting a higher `backend_priority`.

## Adding an operation (checklist)

1. Operation class in `include/xmipp4/em/ops/<category>/` +
   `src/em/ops/<category>/` (reuse the shape/data-type policies from
   `xmipp4::ops` where possible).
2. Functional verb in `include/xmipp4/em/functional/` +
   `src/em/functional/`.
3. Reference CPU builder in `src/backends/cpu/builders/em/<category>/`
   and its registrar entry.
4. Unit tests mirroring each new file under `tests/unitary/src/`, plus an
   integration test driving the verb end to end.
