# Decisions

Judgement calls, written down so the reasoning survives and nobody reopens
them by accident. Each names the condition that would reopen it, so a future
reader can tell a considered choice from an oversight. Implementation work
lives in [BACKLOG.md](BACKLOG.md).

Reviewed 1 Oct 2026. Nothing is currently waiting on an answer.

## Shapes and types

### A shape is a tuple going out and any sequence coming in

`shape` returns a tuple on both `Array` and `ArrayDescriptor`, as numpy, torch
and JAX do. Anywhere extents are taken, any sequence is accepted — pybind11
loads a `std::vector` from anything passing `isinstance<sequence>`, so a list,
a tuple and a `range` all reach `make_contiguous_array_descriptor`. Tests pin
both halves, including the round trip: a shape this reports is one it accepts.

### numpy is not a dependency of the package

`Array` exchanges memory with numpy and needs none of it: the buffer protocol
and DLPack do not import it, and `__array__` imports it when it is called,
which only numpy does. It is a test requirement and nothing else.

**Revisit when** the package needs numpy to compute something rather than to
hand something over.

### `__eq__` without `__hash__` leaves a type unhashable

Where rexlib gives a type a `hash()` of its own, forward it, as
`array_descriptor` does. Where it does not —
`device_index` is the one left — the hash would have to be written rather
than forwarded, and that belongs in rexlib beside the ones that already exist.
`_session_pool.py` keys on a `(backend, id)` tuple in the meantime.

**Revisit when** rexlib gives `device_index` a `hash()`.

### The module path is the area, not the function name

The submodules follow rexlib's directories, not its namespaces. An area under
`em/` sits inside `namespace em` the way `core/hardware/` sits inside
`namespace rexlib`, so it binds as `em.<area>` rather than as `em`, even where
its functions carry no qualifier of their own. The path is what keeps the
areas apart, so that a `read` in one never has to be renamed for the next.

`em` has no area today. Image I/O was the one it had, and it moved to
[vitrio](https://github.com/gigabit-clowns/vitrio).

## Exchanging memory

### Two protocols, and `__array__` beside them

`Array` hands its memory out through the buffer protocol and through DLPack,
and takes memory in through DLPack. Both are needed going out:
`numpy.asarray` and `memoryview` never ask for DLPack, and torch and JAX
never ask for a buffer.

`__array__` is there for the arrays the buffer protocol cannot describe.
numpy asks it last, and without it would wrap such an array in an object
array instead of failing.

`__array_interface__` and `__cuda_array_interface__` are left out. The first
reaches nothing the buffer protocol does not, and the second reaches only
CUDA, which DLPack covers.

**Revisit when** a library that matters speaks only one of the two left out.

### Only host memory crosses, and nothing is moved

An export refuses storage the host cannot reach instead of transferring it.
Any buffer the host can address goes out as DLPack's CPU device, pinned and
unified memory included.

`from_dlpack` takes a tensor on the CPU device and no other. It has no
`device` argument and never passes `dl_device`, so no producer moves data to
satisfy it. `__dlpack__` refuses a `stream`, and a `dl_device` other than the
CPU.

Device memory needs three facts no backend states yet: the DLPack identity of
a memory resource, the device address of a buffer and the native stream of a
queue. rexlib-cuda cannot run a program today, so their signatures would be
guesses.

**Revisit when** rexlib-cuda runs programs. A stream named by a consumer then
becomes a queue, given a command that only waits for what the array is still
used by.

### A crossing waits; what is kept afterwards does not

Every export goes through rexlib's `get_host_data`, which waits for the
commands that still use the array. Nothing waits again after that:

- A view numpy keeps is not synchronised with a command submitted later.
  Tracking it would need an access that ends only when the view dies, and a
  command waiting on a view held by the thread that submitted it would never
  run.
- An array built by `from_dlpack` is not ordered against its source, even
  when the source is another `Array`. rexlib orders arrays that come from one
  another, and these two only share a buffer.

Neither can be observed today: the CPU backend finishes a command before
`submit` returns.

**Revisit when** a backend that runs commands later shares host memory with
them, which pinned and unified memory do.

### A read-only source is copied or refused

Python has no read-only `Array`, so a source that says it is read only is
never shared. `from_dlpack` copies it, which needs an active device since
rexlib allocates the copy, and refuses it under `copy=False`. `copy=True`
asks the source for a copy of its own, as the array API standard has it.

A source that does not say so is shared. JAX is one: its arrays are immutable
and it exports a tensor without a version, which has no flag to carry that.
Writing to such an array is the caller's mistake, as it is from torch.

**Revisit when** `const_array` is bound.

### Memory that is not aligned is refused

numpy exports an array whose data is not aligned for its type, and rexlib's
kernels read typed memory. `from_dlpack` refuses it rather than copy it: the
copy would be a kernel reading that memory.

**Revisit when** a caller meets it with data that cannot be realigned at the
source.

### `char8` is a C `char` to a buffer, and nothing to DLPack

The buffer protocol is given the format of a C `char`, so numpy sees an 8-bit
integer whose signedness is the platform's. DLPack has no character type, and
`char8` is refused in both directions.

`complex_float16` is the reverse: DLPack names it and no buffer format does,
so it leaves through DLPack only.

**Revisit when** arrays of text are used from Python.

### torch and JAX are tested where they are installed

Neither is a test requirement: across five platforms and six Python versions
they would outweigh everything else the suite installs. The tests that need
one are skipped in CI, where numpy and a tensor built by hand cover the
protocol.

**Revisit when** a release of either breaks the exchange unnoticed.

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
