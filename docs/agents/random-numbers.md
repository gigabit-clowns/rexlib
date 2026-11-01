# Random numbers

How operations that draw random numbers take their keys and lay out their
draws.

An operation that draws random numbers takes a key as an input operand. A key
is a uint64 element, and an operation reads it and never writes it. Nothing in
the library holds generator state, and no operation returns a key.

A key broadcasts like any other operand. It gives one stream of draws over
every element of the result it covers. The position of an element in that
stream is its row-major position among the elements that share its key.

`make_key` turns a seed into a key, and holds the seed unchanged.
`fold_in_key` and `split_key` derive new keys; arithmetic on keys does not.
No function picks a seed by itself. `entropy_seed` draws one from the system
when the caller asks for it.

The generator is Philox4x32-10. The low half of the key is word 0 of its key,
and the high half is word 1. Every operation and every backend lays out the
four words of the counter in the same way:

| Words  | Content                                                       |
|--------|---------------------------------------------------------------|
| 0 to 1 | The index of the block in the stream, low half first          |
| 2      | The index of the stream within the operation, 0 for the first |
| 3      | The domain: 0 for draws, 1 for derived keys                   |

The documentation of an operation states the counter of each draw and how
its bits become a value, so that every backend draws the same bits.
