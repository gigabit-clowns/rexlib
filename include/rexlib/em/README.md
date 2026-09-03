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
`image_reader` or `image_writer` exposes the contents of **one image file**
as one rectangular ND array, and every access is a set of hyperrectangles of
it. Element `i` of an `(N,H,W)` stack is the region of extents `(H,W)` at
offset `(i,0,0)`; a patch of an `(H,W)` micrograph is the region of extents
`(h,w)` at offset `(y,x)`. There is one read verb, not one per access
pattern. Random versus sequential is a property of a sequence of requests
rather than of one, so it belongs to tier two and is nothing a reader
distinguishes.

A read takes a whole batch of regions in one `image_transfer_plan` rather than
one region per call. That is the only shape in which a reader can see enough
to sort the accesses by their position on the storage and merge neighbouring
ones, and it keeps the per region cost to arithmetic: the plan holds its
offsets in flat vectors, so reusing one from one call to the next allocates
nothing after the first. Measured on a batch of 256, a call per region cost
110 to 150 ns and four allocations each; building and walking the batched
form costs about 8 ns per region and two allocations for the whole batch.

The plan names its two sides for what they are rather than for which way the
data moves: one is **the file** and the other **the array**. A plan is
therefore built the same way for a read as for a write, and the direction
belongs to the call. Its extents are the shape of one region and carry the
rank of the region alone, so a side of higher rank spans a single position
along the axes they do not reach. A batch of two dimensional patches cut from
a micrograph into a three dimensional array therefore has extents of rank
two, and neither side pads them; ranks may differ in either direction, so a
plane of a stack reads equally into an array that does not carry the axis it
was stacked along.

The offsets live in `index_table`, a flat table of same-rank indices, so a
batch of any size costs a bounded number of allocations and `clear()` keeps
the capacity: one plan reused from one call to the next allocates nothing
after the first. `interned_path_list` is its counterpart for paths, holding
equal paths once and referring to them by index, which is what a transaction
over many files needs when the same stack is named by region after region.

The extents alone do not say what a file holds: `(N,H,W)` is a stack of `N`
images or one volume of `N` planes, and the two are written differently. A
reader and a writer therefore carry a **core rank** beside their extents,
counting the trailing axes that make up one image or volume. It is the
dimensionality of what the file holds, two for images and three for
volumes, so the distinction is a rank rather than a flag, and the axes it
leaves over at the front are the ones the file stacks along. A format
records the difference in its own way, MRC through its space group and its
grid size.

A reader always produces correct data for any in-bounds box, and `read` is
always safe to call concurrently: a format that cannot decode in parallel
serialises the calls itself, so a caller never loses correctness by issuing
them at once. Nothing advertises what a read costs, because with the whole
batch in hand the reader orders and merges its own accesses and such a hint
was left without a consumer.

Formats are registered per action. `image_read_format` and
`image_write_format` are separate interfaces with separate registries and
separate `service_manager`s, so a consumer that only reads never constructs
the write side. Each format `.cpp` ends in a `REXLIB_REGISTER_IMAGE_*_FORMAT`
macro; there is no registrar list to add to.

Tier two spans **many files per transfer**, which is the one thing tier one
does not do. Its unit is a **transaction**: one array, one set of files,
completing as a whole. `image_transaction_plan` describes one, and is the peer
of `image_transfer_plan` rather than a composite of it — the same regions, the
same two sides, with a file named per region.

A plan holds its regions in the order they were added and in no other.
Walking them one file at a time, so that each file is opened and read once,
is `region_grouping`: it names regions rather than moving them, and it is a
type of its own rather than a phase of the plan, because it is what a
consumer wants of a transaction and not something the transaction is.

A path becomes an open handle through an `image_reader_provider` or an
`image_writer_provider`, which do that and nothing else: they transfer
nothing, and they say nothing about where a handle comes from, so a provider
serving readers from a plugin or a shared memory segment fits the same
interface with no format manager in it. On the reading side
`direct_image_reader_provider` opens through the format manager and
`caching_image_reader_provider` is a bounded LRU **decorating another
provider**, so the code that turns a path into a reader lives in one place.

The writing side can not evict, because creating a file replaces it and a
writer dropped and reopened would truncate everything already written. Its
files are therefore said outright: `managed_image_writer_provider::declare`
takes exactly what `image_write_format::open` takes, since that is what it
will eventually be handed, and the file is created when it is first
acquired. `close` flushes it, drops the writer and forgets the declaration
together, so acquiring it afterwards fails rather than replacing the file.
Nothing serialises writes; tier one allows concurrent ones only for regions
that do not overlap, and that is passed up rather than papered over.

The rest of the tier — the sources and sinks, the executor and the scratch
cache — is not implemented yet.

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
