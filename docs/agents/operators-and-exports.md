# Operators and exported symbols

How operators are declared and what crosses the boundary of the shared library.

## Overloaded operators

They are hidden friends: a `friend` declaration inside the class, so that the
operator is reachable only through argument dependent lookup, and both operands
convert alike. The definition goes in the class body too, which is the only
form that also works for templates and for `constexpr`.

The exception is an operator whose body needs a type that is incomplete in the
header — `strided_layout` compares through its implementation, which is only
forward declared — and those keep their definition in the source file. The
`friend` declaration still hides them.

## Exported symbols

The library builds with hidden visibility. `REXLIB_API` marks what crosses the
shared object boundary. On MSVC, a dll-interface class holding a standard
library member warns with C4251, and `REXLIB_STD_MEMBER_INTERFACE` on the line
above silences it for that member alone, leaving the warning active elsewhere.
`REXLIB_STD_BASE_INTERFACE` does the same for C4275 on a base class.

An operator defined in a class body is implicitly inline and needs no
`REXLIB_API`.

The dependencies linked as static libraries do not follow that visibility on
their own: a static library exports whatever it defines. On Linux the shared
library is therefore linked with `--exclude-libs,ALL`, so that boost stays
inside it rather than being resolved against another copy the process holds.
