# Naming

How implementations and functions are named beyond what `.clang-tidy` checks.

## Implementations of an interface

A class that implements an interface, or narrows one for a backend, is named
`<qualifier>_<interface>`, the qualifier saying which one it is: a backend, a
strategy, a kind of memory. `cpu_device` and `cpu_command_queue` implement
`device` and `command_queue`, `cpu_program` narrows `program` for the CPU
backend, and `host_buffer` and `eager_dispatcher` implement `buffer` and
`dispatcher`. Its files carry the same name.

The name stands on its own, without the namespace around it. A backend keeps
its code in a namespace of its own, `rexlib::cpu` here and `rexlib::cuda` in
`plugins/cuda/`, so that its
helpers cannot clash with those of another backend loaded into the same
process; that namespace is never what tells an implementation from its
interface. So the CPU device is `rexlib::cpu::cpu_device`, and inside the
backend a plain `device` is the interface.

## Functions of a fixed number of axes

A function acting along the last two or three axes is the operation that
acts along named axes, with the axes filled in. An established NumPy name
keeps NumPy's spelling, as `fft2` and `fft3` do; any other function takes a
`_2d` or `_3d` suffix, as `standardize_background_2d` does.
