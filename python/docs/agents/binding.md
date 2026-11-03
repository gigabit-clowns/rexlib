# The binding

The conventions of the C++ code under `src/`.

Every bound entity gets a `.hpp` and a `.cpp`. The header declares a class
alias and the two functions the source defines:

```cpp
using device_class = nanobind::class_<device, PyDevice>;

device_class declare_device(nanobind::module_ &m);
void define_device(device_class &c);
```

`declare_X` creates the type and nothing else. `define_X` fills it in. A
namespace's `main.cpp` calls every `declare_` first and every `define_`
afterwards, which is what lets any binding refer to any other type regardless
of the order they appear in — a default argument, a return type, a base class.
Splitting the two is the whole point; do not collapse them because a
particular type happens not to need it today.

`define_X` takes a second `nanobind::module_ &` when it also registers
something at module level, which is how the free functions beside a type are
bound: `define_device_manager` adds `get_device_manager`, `define_version`
adds the `rexlib_version` attribute.

A source declares `namespace nb = nanobind;` inside `namespace rexlib` and
writes `nb::` throughout, signatures included. A header writes `nanobind::`
in full, since an alias there would reach every source that includes it.

Managers are reached through a module-level getter taking a `ServiceCatalog`,
never constructed: `get_device_manager(catalog)`, `get_program_manager(catalog)`.
They are services the catalog owns.

Names follow rexlib on the C++ side — `lower_case`, `m_` on private members —
and Python on the bound side: `CamelCase` types, `lower_case` functions and
properties. A C++ getter pair becomes a property, not `get_x()`/`set_x()`.

Every argument is named with `nb::arg`. An operation is bound by pointer,
straight from rexlib, with its optional out parameter, the `array *` rexlib
takes, as `nb::arg("out").none() = nb::none()`; nanobind hands `None` in as
`nullptr`. The `const_array_ref` and `array_ref` rexlib takes arrive through
`array_ref_caster.hpp`. A function of the binding's own sits between only
where something has to be converted, as the fill value of `full` is.

A type caster for a standard type — `<nanobind/stl/string.h>`,
`shared_ptr.h`, `vector.h`, `optional.h` and the like — is included by every
source that binds a signature carrying that type, written out or reached
through a member pointer. Without it nanobind still compiles, taking the type
for a bound one, and fails at run time. There are no holder types: a
`shared_ptr` to a bound type crosses through `shared_ptr.h`.

A type that defines `__eq__` and no `__hash__` should be unhashable, so that
it cannot be a dictionary key or go in a set, as Python makes such a class.
nanobind does not: the type would hash by identity, and two equal values would
be two keys. Where rexlib gives the type a `hash()` of its own, as
`array_descriptor` does, forward it. Where it does not, set `__hash__` to
`None`, as `Version` and `DeviceIndex` do. Writing the hash is rexlib's to do
rather than this binding's; `_session_pool.py` keys on a `(backend, id)` tuple
in the meantime.

A binding that blocks releases the GIL: held through the wait, it would
freeze every other Python thread for its duration. Where the whole function
blocks, that is `nb::call_guard<nb::gil_scoped_release>()`. nanobind converts
the arguments before the guard is constructed and the return value after it is
destroyed, so nothing touches Python without it.

Where only part of a function blocks, the release is a
`nb::gil_scoped_release` in a block around that part. Handing out an array's
memory is the case: it waits for the commands that still use the array, and
then builds a Python object.

`rexlib::array` is move-only, and anything taking one by value has to be given
`destination.share()` rather than the caller's own, or `source.share_const()`
where it takes a `const_array`. Bound plainly, the binding would move the
array out of the Python object and hand the caller back an empty one.

A type rexlib parses from a string carries a `from_string` static method and
no constructor that parses, as `DeviceIndex` does. A constructor is left free
to give a lone string another meaning, and a parsing overload would compete
for that signature rather than add to it. `from_string` is the inverse of
`__str__` and raises `ValueError`, which is what the `bool` of rexlib's
`parse_*` becomes on this side.

Lifetimes are stated: `nb::keep_alive` where an object borrows from another,
`nb::rv_policy::reference_internal` where a getter hands out a
reference into its owner. Getting this wrong does not fail a test, it crashes
a user.

A rexlib type with no Python counterpart gets a `type_caster`, not a bound
class — see `float16_caster.hpp`, which converts `float16_t` through `float`,
and `array_ref_caster.hpp`, which loads a reference to an array from an
`Array` and gives it `Array`'s name in the stubs.

`PyX` trampolines, declared with `NB_TRAMPOLINE`, live in the header, not the
source, because they are part of the `class_` alias callers see.

`Array`'s buffer protocol is a pair of type slots in `array.cpp`, nanobind
binding no buffer of its own. They check the layout a consumer asks for, and
keep what the view points to besides the memory in its `internal` field.
