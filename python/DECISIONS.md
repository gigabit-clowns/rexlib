# Decisions

Judgement calls, written down so the reasoning survives and nobody reopens
them by accident. Each names the condition that would reopen it, so a future
reader can tell a considered choice from an oversight. Implementation work
lives in [BACKLOG.md](BACKLOG.md).

Reviewed 1 Oct 2026. Nothing is currently waiting on an answer.

## Loaders and savers

### A destination's data type is chosen, not discovered

`image_reader::read` converts values to the data type of the destination with
`numerical_cast` semantics, so a caller may ask for whatever type the rest of
the pipeline computes in and the read converts on the way. The type is fixed
by what the work needs rather than by what the file happens to hold, and
reading straight into it saves a separate conversion pass later.

The conversion is the design rather than an oversight. A file's own type is
still there for whoever wants it: `query_descriptor` reports it, and the
synchronous `read` keeps it unless it is asked for another.

**Revisit when** a pipeline is measured to be spending materially in that cast
and reading into the file's own type would avoid it.

### The thread pool is per instance

Sharing an executor between loaders is the caller's responsibility rather than
the binding's; most clients hold a single loader and a single saver. A default
is still provided, and per instance is the one chosen, with an explicit
executor accepted as an override. `loader()` and `saver()` both follow it.

Per instance rather than process-wide because the choice is meant to be
revisited once there is evidence of how clients use it, and this is the
direction that is cheap to reverse: moving to a shared pool later takes
nothing away, while moving from a shared pool to per-instance would change
behaviour anyone had come to rely on.

**Revisit when** there is evidence of clients holding several sources and
oversubscribing their cores.

### The completion is mirrored, not wrapped

Layer one exposes `wait`, `is_ready` and `get` as they are. Layer two does not
wrap them in `concurrent.futures` or `await`.

A pipeline already works by ordering N batches and collecting them; the
parallelism lives in the C++ thread pool rather than in Python's concurrency
model. Both wrappers need a Python thread per outstanding completion to
bridge, which adds overhead to the thing being made fast, and the completion
carries no value — `get` returns void and only rethrows — so a `Future[None]`
buys little. It stays additive: a `to_future()` can arrive later without
changing what exists.

**Revisit when** a caller needs to select over completions, or to integrate
with code already built on `asyncio` or `concurrent.futures`.

### Blocking calls release the GIL

`wait` and `get` block, and the asynchronous reads and writes open files
through their provider before returning. Bound plainly they would hold the
GIL and freeze every Python thread for the duration, which makes the
asynchronous interface pointless. All of them take
`py::call_guard<py::gil_scoped_release>()`, and so does everything else in
`em.image` that reaches a file: the synchronous reads and writes,
`query_descriptor`, `flush` and `close`.

Safe because pybind11 constructs the guard after the arguments are converted
and destroys it before the return value is cast, so nothing touches Python
without the GIL.

**Not to be reopened.** This is the difference between the class working and
being decorative.

## Shapes and types

### A shape is a tuple going out and any sequence coming in

`shape` returns a tuple on both `Array` and `ArrayDescriptor`, as numpy, torch
and JAX do. Anywhere extents are taken, any sequence is accepted — pybind11
loads a `std::vector` from anything passing `isinstance<sequence>`, so a list,
a tuple and a `range` all reach `make_contiguous_array_descriptor`. Tests pin
both halves, including the round trip: a shape this reports is one it accepts.

### Centres are an `IndexTable` in the binding and any sequence in the package

`read_patches_async` takes an `index_table` in rexlib, so the binding takes an
`IndexTable` and nothing else. The package accepts any sequence of sequences
beside it and builds the table, with the rank the destination implies, which
is what keeps an empty list of centres meaningful. It also accepts a numpy
array, which it hands to `IndexTable.from_array`.

The table is bound as a class rather than hidden behind a caster because a
caller cropping many batches can fill one and reuse it, which a conversion on
every call would not allow.

### An `IndexTable` and a numpy array convert into each other in C++

`IndexTable.from_array(indices)` takes an array of integers with one row per
index, and `numpy.asarray(table)` gives one back through `__array__`. Neither
has a counterpart in rexlib. They are in the binding because that is the only
place where the rows are copied without crossing into C++ once per index,
which is some thirty times slower for a table filled from Python with `add`.

It is a static method rather than a constructor because the constructor
already gives a lone argument the meaning "this rank". An overload told apart
only by the type of that argument is the reason `ImageLocation` has
`from_string`.

`from_array` takes a `numpy.ndarray` and nothing that merely converts to one:
not a list, not a buffer, not an object with `__array__`. Converting is one
`numpy.asarray` away, and for some of those objects it moves data off a
device or runs a deferred computation, which this package never does on a
caller's behalf. It is also what pybind11 can state: the stub says
`numpy.ndarray`, and that is exactly what is accepted. An array that does not
hold integers is refused, and so is one holding a negative index, which would
otherwise wrap around into a huge one.

**Revisit when** callers routinely hold their centres in something else, a
torch tensor or a data frame, and the `numpy.asarray` is noise.

The array a table becomes is a copy, of `numpy.uintp` unless another type is
asked for. A view would be left dangling by the next index added, so
`copy=False` is refused.

numpy is not a dependency of the package: it is reached only when a caller
hands an array over or asks for one, and whoever does has it. It is a build
requirement, since pybind11-stubgen has to import it to name it in the stubs,
and a test requirement.

**Revisit when** `Array` exchanges data with numpy, which would make numpy a
dependency in its own right.

### Only what the high-level functions need is bound

`rexlib.em.image` exposes the functions of `image_read.hpp` and
`image_write.hpp` and the classes a caller has to construct to call them.
Readers, writers, formats, transfer plans and sanitizers stay in C++: nothing
Python does with them could not be done through those functions, and each
would be another surface to keep matching rexlib.

**Revisit when** a format or a border rule has to be written in Python.

### `__eq__` without `__hash__` leaves a type unhashable

Where rexlib gives a type a `hash()` of its own, forward it; `array_descriptor`,
`image_descriptor` and `image_location` all do. Where it does not —
`device_index` is the one left — the hash would have to be written rather
than forwarded, and that belongs in rexlib beside the ones that already exist.
`_session_pool.py` keys on a `(backend, id)` tuple in the meantime.

**Revisit when** rexlib gives `device_index` a `hash()`.

### `image_metadata` is not bound

rexlib declares the class empty and says so. Leaving the parameter off the
writes and off `ManagedImageWriterProvider.declare` until the type has fields
is backward compatible; publishing an empty class whose shape is going to
change is not.

**Revisit when** rexlib gives `image_metadata` contents.

### Python never parses a location out of a string

A path handed to `read` or to `ImageLocation` is a path. `"3@stack.mrc"`
becomes a position in a stack only through `ImageLocation.from_string`, asked
for by name. The C++ overload behaves the same way; if the convenience is ever
wanted it is a change in rexlib, not in the binding.

A type rexlib parses from a string carries a `from_string` static method and
no constructor that parses. `ImageLocation` is why: its constructor already
gives a lone string the meaning "this path, whole file", so a parsing overload
would compete for that signature rather than add to it. `from_string` is the
inverse of `__str__` and raises `ValueError`, which is what the `bool` of
rexlib's `parse_*` becomes on this side.

### `em.image.read` requires an active device

Reading a file into host memory needs no device, yet `read` raises outside
`with rexlib.device(…)` because it goes through the same `_resolve_context` as
`zeros`. Coherence won over convenience: the day that rule stops making sense
it should stop for everything, which is a change in one function rather than
an exception carved out for file I/O. `em/image/_functions.py` imports that
resolver rather than copying it, so the coupling is real and not merely
similar.

**Revisit when** `_resolve_context`'s rule is reconsidered for the package as
a whole.

### The module path is the area, not the function name

The submodules follow rexlib's directories, not its namespaces. `em/image/`
sits inside `namespace em` the way `core/hardware/` sits inside
`namespace rexlib`, so it binds as `em.image` rather than as `em`, even though
`em::read` carries no `image` qualifier of its own. The path is what keeps
images apart from the areas that follow — metadata, particle formats — so that
`read` never has to become `read_image`.

File formats are not that axis: EER, MRC and TIFF register with the format
manager and arrive through the same `em.image.read` with no API change.

## Build and CI

### A kept build directory is offered, not imposed

`SKBUILD_BUILD_DIR` makes a repeated `pip install .` incremental instead of a
full rebuild, and the README documents it as an opt-in rather than
`pyproject.toml` setting it for everyone. The gain belongs to developers
alone: a user installing from an sdist builds in a directory pip deletes
afterwards. The cost does not stay with developers — a CMake cache that
outlives the options it was configured with would reach every user, and it
would move six wheel jobs that already hit their compiler cache without it.

The value is `build/binding`, not a bare `build`, because the README has
rexlib built into `build/rexlib` a few lines earlier and the advice to delete
the directory would otherwise take rexlib's build with it.

The same setting is used by the source distribution check, where it took the
job from 16m52s to 38s: scikit-build-core otherwise builds in a temporary
directory unrelated to the one pip unpacks into, so the sources sit somewhere
different relative to the build directory on every run and `CCACHE_BASEDIR`
has nothing stable to rewrite against. The value has to stay relative, since
it is resolved against the source directory.

**Revisit when** the local loop is measured and the default costs more than
the sticky cache would.
