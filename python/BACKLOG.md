# Backlog

Work whose shape is settled and only waits to be scheduled. Everything here
has a known method; nothing here is waiting on a judgement call. Open
questions live in [DECISIONS.md](DECISIONS.md).

Reviewed 20 Sep 2026, against rexlib-python `5839243` and rexlib `43bbffec`.

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

## 3. Bind the patch source

`image_patch_source` arrived with the batch source upstream and crops batches
of patches out of an image, clipped at the borders. The batch source is bound;
this one is not, and it stands on the same chain, so it is a smaller piece of
work than the first was.

## 4. Translate the library's exceptions

There is no `py::register_exception` anywhere in `src/`, so everything rexlib
throws arrives through pybind11's catch-all. `invalid_operation_error` derives
from `std::logic_error` and `image_format_error` from `std::runtime_error`,
neither of which pybind11 special-cases, so both land as a bare
`RuntimeError`.

This cost nothing while the surface a caller could fail against was small. It
no longer is: a missing file, a format nothing recognises, a truncated file
and an array the host cannot reach are four different mistakes that a caller
meets routinely through `em.image` and cannot tell apart.

The cheapest visible win is a missing file arriving as `FileNotFoundError`. It
reaches `image_read_format_manager::open` as "no registered format recognizes
the file", which is true but is not what went wrong and is not what anyone
will search for.

Register the types once where the module is built so every submodule inherits
them; translate only what genuinely corresponds to a Python builtin and give
the rest types of their own; the tests under `tests/em/image/` currently
assert `RuntimeError` and follow.

## 5. Zero-copy exchange with the array ecosystem

`Array` can now be measured and printed, but nothing can be handed to anything
else: there is no buffer protocol, no `__array__`, no DLPack. Loading batches
at speed and receiving objects that cannot leave the package is a pipeline
without an exit.

Worth separating: the buffer protocol or `__array__` for host-resident arrays,
where device-resident ones refuse rather than transfer, matching the explicit
transfer rule the library holds everywhere else; and DLPack afterwards, if
interoperating with torch or cupy becomes a goal, since it is the right
protocol for device memory and the wrong place to start.
