# em — CryoEM domain component

This directory holds the CryoEM-specific operation set and the image I/O
subsystem. It follows the same conventions as the generic components and
must be kept in sync with them.

## Layout

```
include/rexlib/em/
├── ops/<category>/     CryoEM operation definitions
│                       (e.g. ops/projection/project_3d_operation.hpp)
├── functional/         user-facing verbs (e.g. project_3d(...))
└── image/              image I/O interfaces and value types
src/em/
├── ops/<category>/     operation implementations
├── functional/         verb implementations
└── image/              I/O implementations, formats/<fmt>/ per file format
```

## Namespace

Everything in this component lives in `rexlib::em` (one namespace per
component). Operations, verbs and image I/O share it, and the directories
group without nesting the namespace: `em::project_3d_operation`,
`em::project_3d(...)`, `em::image_reader`.

## Dependencies

`em` may depend on `core`, `ops` and `functional`. `core`, `ops` and
`functional` must never include `rexlib/em/` headers; backends and plugins
may, since that is how they implement em builders and formats. Keep the
arrows pointing downwards.

## Backend builders

Every backend implements its `program_builder`s for em operations under
`builders/em/<category>/`, mirroring the catalog:

- in-tree CPU: `src/backends/cpu/builders/em/<category>/`
- plugin repositories (e.g. rexlib-cuda) mirror the same skeleton and
  expose their builder SDK as `<rexlib/backends/<name>/...>`.

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
`REXLIB_REGISTER_*_PROGRAM_BUILDER` macro, which registers it at static
initialization; there is no registrar list to add to.

## Coverage policy

The in-tree CPU builders are the reference tier: every em operation must
have a (possibly naive) CPU builder before it is merged. Accelerator
backends are best effort; optimized plugins may override the reference
builders by reporting a higher `backend_priority`.

## Image I/O

`image/` is a two-tier subsystem. Tier one is synchronous and per file: an
`image_reader` or `image_writer` exposes exactly **one rectangular ND
dataset**, and every access is a set of hyperrectangles of it. Element `i` of
an `(N,H,W)` stack is the box at offset `(i,0,0)` with extents `(1,H,W)`; a
patch of an `(H,W)` micrograph is the box at `(y,x)` with extents `(h,w)`.
There is one read verb, not one per access pattern. Random versus sequential
is a property of a sequence of requests, so it belongs to tier two and to the
access hint given when the file is opened.

A read takes a whole batch of regions in one `image_region_list` rather than
one region per call. That is the only shape in which a reader can see enough
to sort the accesses by their position on the storage and merge neighbouring
ones, and it keeps the per region cost to arithmetic: the list holds its
offsets in flat vectors, so reusing one across batches allocates nothing
after the first. Measured on a batch of 256, a call per region cost 110 to
150 ns and four allocations each; building and walking the batched form
costs about 8 ns per region and two allocations for the whole batch.

A reader always produces correct data for any in-bounds box; what varies
between formats is the price, which `image_access_traits` advertises. Thread
safety follows the same rule: `read` is always safe to call
concurrently, and the `concurrent_read` flag says whether doing so buys
throughput, never whether it is allowed.

Formats are registered per action. `image_read_format` and
`image_write_format` are separate interfaces with separate registries and
separate `service_manager`s, so a consumer that only reads never constructs
the write side. Each format `.cpp` ends in a `REXLIB_REGISTER_IMAGE_*_FORMAT`
macro; there is no registrar list to add to.

Tier two — asynchronous batching, scheduling and the scratch cache — is not
implemented yet.

## Adding an operation (checklist)

1. Operation class in `include/rexlib/em/ops/<category>/` +
   `src/em/ops/<category>/` (reuse the shape/data-type policies from
   `rexlib::ops` where possible).
2. Functional verb in `include/rexlib/em/functional/` +
   `src/em/functional/`.
3. Reference CPU builder in `src/backends/cpu/builders/em/<category>/`
   and its registrar entry.
4. Unit tests mirroring each new file under `tests/unitary/src/`, plus an
   integration test driving the verb end to end.
