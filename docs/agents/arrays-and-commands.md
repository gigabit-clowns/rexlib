# Arrays and the commands that access them

How commands on the memory of an array are ordered.

A queue runs a command after the commands it depends on and promises no other
order, so two commands that touch the same memory have to be ordered by a
dependency. Arrays keep the record that makes this automatic: every array
carries an `access_hazard_tracker` holding the tokens of the commands that
still write or read its memory. A dispatcher takes the dependencies of a
command from the trackers of its operands, and records the token it gets back
on them.

Three rules follow for anything that touches the memory of an array:

- An array over the memory of another is built from that array, through the
  constructor of `array_implementation` that takes one, never from
  `share_storage()`. That is what makes the two share a tracker; `subarray`
  and the complex views do it. Two arrays built separately over one buffer are
  not ordered against each other.
- The host reaches an array through `get_host_data`, which waits for the
  commands in its way. `buffer::get_host_ptr` waits for nothing.
- Memory an allocator hands out comes with a token. Whoever allocates gives it
  to the tracker of the new array, or to the dependencies of the command when
  the buffer belongs to no array.
