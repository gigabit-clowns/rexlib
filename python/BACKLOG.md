# Backlog

Work whose shape is settled and only waits to be scheduled. Everything here
has a known method; nothing here is waiting on a judgement call. Open
questions live in [DECISIONS.md](DECISIONS.md).

Reviewed 1 Oct 2026, against rexlib-python `ea1b87d` and rexlib `09f73158`.

## 1. Bind the rest of `functional`

Four of rexlib's seventeen `functional` headers are bound — `arithmetic`,
`cast`, `creation` and `transfer` — and two of those only partly. Thirty-one
names reach Python; roughly ninety-seven do not.

This is the largest gap in the package and what most of the rest leans on. It
is also the cheapest work here: each block is a mechanical translation against
a template that exists twice over, in `src/functional/arithmetic.cpp` and
`python/rexlib/_functional.py`.

Sequenced by what each one unblocks rather than by size:

| Step | Header | Functions | Unblocks |
|---|---|---|---|
| 1 | `compare.hpp` | 6 | `__eq__` … `__ge__` |
| 2 | `power.hpp` | 12 | `__pow__`, `__ipow__` |
| 3 | `bitwise.hpp` | 6 | six bitwise dunders and their in-place forms |
| 4 | `view.hpp` | 1 | `__getitem__`, `__setitem__` |
| 5 | `reduction.hpp` | 10 | aggregates |
| 6 | `linalg.hpp` | 5 | `__matmul__`, `__imatmul__` |
| 7 | `rounding`, `selection`, `numeric`, `complex`, `logical` | 21 | — |
| 8 | `trigonometric.hpp` | 17 | — |
| 9 | `fourier.hpp` | 14 | the CryoEM use case |

Holes in the headers already bound: `divmod`, `floor_divide` and `sign` in
`arithmetic`, the first two of which `__divmod__` and `__floordiv__` need; and
`arange` and `linspace` in `creation`.

Per block: a `.cpp`/`.hpp` pair with every argument required and an optional
out parameter as `std::optional<array*>` defaulting to `py::none()`; a section
in `_functional.py` where `context` falls back to the active one; a re-export
with `as` plus a line in `__all__`; a file under `tests/functional/`.

## 2. Finish `Array`'s Python operators

Tracked as issue #115. `python/rexlib/_ndarray.py` installs fifteen of roughly
forty: the five arithmetic binaries, their in-place forms, `__neg__`,
`__abs__`, `__pos__`, `__copy__` and `__deepcopy__`.

This is the ceiling of item 1 rather than work to schedule on its own. Each
block lands, the operators it affords go in with it, and the issue closes when
the ladder is climbed.

One trap before `compare` lands: a type that defines `__eq__` and no
`__hash__` is unhashable. `ArrayDescriptor` forwards rexlib's
`array_descriptor::hash()` for this reason. `Array` has no such member in
rexlib, so the comparison operators need an answer about identity before they
go in.

## 3. Translate the library's exceptions

There is no `py::register_exception` anywhere in `src/`, so everything rexlib
throws arrives through pybind11's catch-all. `image_file_error`,
`image_format_error`, `unsupported_operation_error` and
`unsupported_capability_error` all derive from `std::runtime_error`, which
pybind11 does not special-case, so all four land as a bare `RuntimeError`.

rexlib now tells them apart, and the binding throws that away: a file that
cannot be reached, a file whose contents contradict its format, a format
nothing recognises and an array the host cannot reach are four different
mistakes that a caller meets routinely through `em.image` and cannot tell
apart. What already arrives well is what rexlib throws as a standard
exception: an index past the end of a stack is an `IndexError`, and a
destination of the wrong shape a `ValueError`.

The cheapest visible win is a missing file arriving as `FileNotFoundError`. It
is an `image_file_error` in rexlib, raised with the path at the front of its
message, and a `RuntimeError` here.

Register the types once where the module is built so every submodule inherits
them; translate only what genuinely corresponds to a Python builtin and give
the rest types of their own; the tests under `tests/em/image/` currently
assert `RuntimeError` and follow.

## 4. Zero-copy exchange with the array ecosystem

`Array` can now be measured and printed, but nothing can be handed to anything
else: there is no buffer protocol, no `__array__`, no DLPack. Loading batches
at speed and receiving objects that cannot leave the package is a pipeline
without an exit.

It is also why the tests under `tests/em/image/` assert shapes, types and
failures and never a value: a patch clipped at a border, or a batch read back
from a stack, cannot be looked into from Python.

Worth separating: the buffer protocol or `__array__` for host-resident arrays,
where device-resident ones refuse rather than transfer, matching the explicit
transfer rule the library holds everywhere else; and DLPack afterwards, if
interoperating with torch or cupy becomes a goal, since it is the right
protocol for device memory and the wrong place to start.
