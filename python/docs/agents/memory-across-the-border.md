# Memory that crosses the border

How memory passes between rexlib and Python: the buffer protocol and DLPack.

The host reaches an array's memory through rexlib's `get_host_data`, never
through `get_storage()->get_host_ptr()`. `get_host_data` waits for the
commands that still use the array and refuses storage the host cannot reach.
The buffer protocol and `to_dlpack` both go through it.

Only host memory crosses, in both directions, and nothing is moved or copied
on a caller's behalf. The one exception is a read-only source, which
`from_dlpack` copies because an `Array` can always be written to.

A capsule handed out owns a `shared_ptr` to the buffer, not a reference to the
`Array`. Its deleter therefore touches no Python object and runs without the
GIL, on whatever thread the consumer calls it from.

Memory taken in is held by `dlpack_buffer`, a `rexlib::buffer` the binding
implements. It is the one class in `src/` that binds nothing. rexlib has no
call that adopts foreign memory and needs none: `buffer` is the interface,
and whoever holds the memory implements it.

A capsule is renamed once its tensor is taken, and only then. Everything that
can refuse a tensor runs before, so a refused tensor stays with its capsule
and is released by it.

A capsule is created with `PyCapsule_New`, not `nb::capsule`. The cleanup of
the latter is handed the pointer whatever the capsule's name, and would
release a tensor a consumer has already taken.

The binding builds with `-Wall -Wextra -Wpedantic`, `/W4 /WX` on MSVC.
nanobind's headers and Python's are system headers to it
(`NB_SUPPRESS_WARNINGS`), so the warnings are the binding's own.
