// SPDX-License-Identifier: GPL-3.0-only

// Tuning knobs of the image subsystem. Every one of them has a default that is
// reasonable on an ordinary machine and may be overridden from the build,
// which is what makes them worth defining rather than writing down at the
// point of use.

// Widest gap between two stretches of a file that is still worth asking for
// in one piece rather than leaving to the fault that reaches it.
//
// The readahead a kernel does of its own accord is about this wide, so
// bridging a gap wider than it is speculation. What a batch actually bridges
// scales with what one of its regions spans and is only capped here, so that
// a gap wider than the data on either side of it is never bridged: a flat cap
// would have two small regions far apart ask for many times what they hold.
#ifndef REXLIB_PREFETCH_GAP_CAP
	#define REXLIB_PREFETCH_GAP_CAP 131072UL
#endif

// Bytes one step of a read asks for before the regions of the step before it
// are walked.
//
// A batch that fits in this is asked for in one go, which is what a batch of
// small scattered regions wants: all of it is in flight before any of it is
// read, so every latency overlaps. Past it a batch is walked a step at a
// time, so that a read whose regions are large enough that asking for all of
// them at once would be asking for more than can be held asks for them as it
// reaches them instead.
//
// This is per read, so several reads at once hold several of these.
#ifndef REXLIB_PREFETCH_BYTE_BUDGET
	#define REXLIB_PREFETCH_BYTE_BUDGET 67108864UL
#endif
