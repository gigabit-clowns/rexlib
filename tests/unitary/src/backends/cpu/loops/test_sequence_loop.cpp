// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/sequence_loop.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

/**
 * @brief Generator writing the index it was handed.
 *
 * Recording the index rather than a value derived from it is what lets a
 * case tell apart where an element went from what the loop thought it was.
 */
struct index_writer
{
	void operator()(std::ptrdiff_t *destination, std::size_t index) const
	{
		*destination = static_cast<std::ptrdiff_t>(index);
	}
};

} // anonymous namespace


TEST_CASE(
	"run_sequence_loop should write every element of a contiguous operand",
	"[sequence_loop]"
)
{
	std::vector<std::ptrdiff_t> destination(5, -1);

	run_sequence_loop(index_writer(), destination.data(), 5, 1);

	CHECK( destination == std::vector<std::ptrdiff_t>{ 0, 1, 2, 3, 4 } );
}

TEST_CASE(
	"run_sequence_loop should count from the first element whatever the "
	"stride",
	"[sequence_loop]"
)
{
	// Every other element is written, and the ones in between are left
	// alone. The index the generator sees is still the position within the
	// sequence, not the offset in memory.
	std::vector<std::ptrdiff_t> destination(7, -1);

	run_sequence_loop(index_writer(), destination.data(), 3, 3);

	CHECK( destination ==
	       std::vector<std::ptrdiff_t>{ 0, -1, -1, 1, -1, -1, 2 } );
}

TEST_CASE(
	"run_sequence_loop should walk backwards on a negative stride",
	"[sequence_loop]"
)
{
	// A reversed view of an array is a negative stride from its last
	// element, so the sequence lands in memory back to front.
	std::vector<std::ptrdiff_t> destination(4, -1);

	run_sequence_loop(index_writer(), destination.data() + 3, 4, -1);

	CHECK( destination == std::vector<std::ptrdiff_t>{ 3, 2, 1, 0 } );
}

TEST_CASE(
	"run_sequence_loop should write nothing when the sequence is empty",
	"[sequence_loop]"
)
{
	std::vector<std::ptrdiff_t> destination(3, -1);

	run_sequence_loop(index_writer(), destination.data(), 0, 1);

	CHECK( destination == std::vector<std::ptrdiff_t>{ -1, -1, -1 } );
}

TEST_CASE(
	"run_sequence_loop should invoke the generator exactly once per element",
	"[sequence_loop]"
)
{
	// A stateful generator, to pin that no element is visited twice on the
	// way to the right contents.
	std::vector<std::size_t> visits;
	const auto recorder =
		[&visits](std::ptrdiff_t *destination, std::size_t index)
		{
			visits.push_back(index);
			*destination = 0;
		};

	std::vector<std::ptrdiff_t> destination(4, -1);
	run_sequence_loop(recorder, destination.data(), 4, 1);

	CHECK( visits == std::vector<std::size_t>{ 0, 1, 2, 3 } );
}
