// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <rexlib/core/layout/joint_layout_builder.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/layout/joint_cursor.hpp>
#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/span.hpp>
#include <core/layout/joint_layout_implementation.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

using namespace rexlib;

namespace
{

using offset_tuple = std::vector<std::ptrdiff_t>;

struct operand_description
{
	std::vector<std::ptrdiff_t> strides;
	std::ptrdiff_t offset;
};

struct layout_description
{
	std::vector<std::size_t> extents;
	std::vector<operand_description> operands;
};

const joint_layout_build_flags reordering_only =
	joint_layout_build_flag_bits::enable_reordering;

const joint_layout_build_flags coalescing_only =
	joint_layout_build_flag_bits::enable_coalescing;

const joint_layout_build_flags reordering_and_coalescing = {
	joint_layout_build_flag_bits::enable_reordering,
	joint_layout_build_flag_bits::enable_coalescing
};

joint_layout build_layout(
	const layout_description &description,
	joint_layout_build_flags flags
)
{
	joint_layout_builder builder;
	builder.set_extents(make_span(description.extents));
	for (const auto &operand : description.operands)
	{
		builder.add_operand(
			make_span(description.extents),
			make_span(operand.strides),
			operand.offset
		);
	}
	return builder.build(flags);
}

std::vector<std::ptrdiff_t> to_vector(span<const std::ptrdiff_t> values)
{
	return std::vector<std::ptrdiff_t>(values.begin(), values.end());
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

/**
 * @brief Every tuple of operand offsets a description holds, sorted.
 *
 * Enumerated from the description rather than through a layout, so that it
 * is what any traversal of a layout built from it has to visit.
 */
std::vector<offset_tuple> enumerate_offsets(
	const layout_description &description
)
{
	const auto &extents = description.extents;

	std::size_t count = 1;
	for (const auto extent : extents)
	{
		count *= extent;
	}

	std::vector<offset_tuple> result;
	std::vector<std::size_t> index(extents.size(), 0);
	for (std::size_t element = 0; element < count; ++element)
	{
		offset_tuple offsets;
		for (const auto &operand : description.operands)
		{
			auto offset = operand.offset;
			for (std::size_t axis = 0; axis < extents.size(); ++axis)
			{
				offset +=
					static_cast<std::ptrdiff_t>(index[axis]) *
					operand.strides[axis];
			}
			offsets.push_back(offset);
		}
		result.push_back(std::move(offsets));

		for (std::size_t axis = 0; axis < extents.size(); ++axis)
		{
			if (++index[axis] < extents[axis])
			{
				break;
			}
			index[axis] = 0;
		}
	}

	std::sort(result.begin(), result.end());
	return result;
}

/**
 * @brief Every tuple of operand offsets a traversal of a layout visits,
 * sorted.
 */
std::vector<offset_tuple> visit_offsets(const joint_layout &layout)
{
	const auto operand_count = layout.get_number_of_operands();

	std::vector<std::ptrdiff_t> inner_strides;
	for (std::size_t operand = 0; operand < operand_count; ++operand)
	{
		const auto strides = layout.get_strides(operand);
		inner_strides.push_back(strides.empty() ? 0 : strides[0]);
	}

	std::vector<offset_tuple> result;
	joint_cursor cursor;
	auto run = layout.iter(cursor);
	while (run > 0)
	{
		const auto offsets = cursor.get_offsets();
		for (std::size_t element = 0; element < run; ++element)
		{
			offset_tuple tuple(operand_count);
			for (std::size_t operand = 0; operand < operand_count; ++operand)
			{
				tuple[operand] =
					offsets[operand] +
					static_cast<std::ptrdiff_t>(element)*inner_strides[operand];
			}
			result.push_back(std::move(tuple));
		}

		run = layout.next(cursor, run);
	}

	std::sort(result.begin(), result.end());
	return result;
}

/**
 * @brief The extent of every axis next to the stride of every operand along
 * it, one entry per axis, sorted.
 */
std::vector<std::vector<std::ptrdiff_t>> axis_columns(
	const joint_layout &layout
)
{
	const auto extents = layout.get_extents();

	std::vector<std::vector<std::ptrdiff_t>> result;
	for (std::size_t axis = 0; axis < extents.size(); ++axis)
	{
		std::vector<std::ptrdiff_t> column = {
			static_cast<std::ptrdiff_t>(extents[axis])
		};
		for (
			std::size_t operand = 0;
			operand < layout.get_number_of_operands();
			++operand
		)
		{
			column.push_back(layout.get_strides(operand)[axis]);
		}
		result.push_back(std::move(column));
	}

	std::sort(result.begin(), result.end());
	return result;
}

/**
 * @brief Strides of a contiguous array whose axes are laid out in a shuffled
 * order, and walked backwards a quarter of the time.
 */
std::vector<std::ptrdiff_t> make_contiguous_strides(
	const std::vector<std::size_t> &extents,
	std::mt19937 &generator
)
{
	std::vector<std::size_t> order(extents.size());
	std::iota(order.begin(), order.end(), std::size_t(0));
	std::shuffle(order.begin(), order.end(), generator);

	std::bernoulli_distribution backwards(0.25);
	const std::ptrdiff_t sign = backwards(generator) ? -1 : 1;

	std::vector<std::ptrdiff_t> strides(extents.size());
	std::ptrdiff_t stride = 1;
	for (auto axis = order.crbegin(); axis != order.crend(); ++axis)
	{
		strides[*axis] = sign*stride;
		stride *= static_cast<std::ptrdiff_t>(
			std::max<std::size_t>(extents[*axis], 1)
		);
	}
	return strides;
}

/**
 * @brief A layout of up to four short axes and up to three operands, some of
 * them contiguous arrays laid out in any order and the rest strided at
 * random, broadcast axes included.
 */
layout_description make_random_description(unsigned seed)
{
	std::mt19937 generator(seed);
	std::uniform_int_distribution<std::size_t> rank_distribution(0, 4);
	std::uniform_int_distribution<std::size_t> extent_distribution(0, 15);
	std::uniform_int_distribution<std::size_t> operand_distribution(1, 3);
	std::uniform_int_distribution<std::ptrdiff_t> stride_distribution(-7, 7);
	std::uniform_int_distribution<std::ptrdiff_t> offset_distribution(0, 99);
	std::bernoulli_distribution contiguous(0.5);

	layout_description description;

	const auto rank = rank_distribution(generator);
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		// Mostly axes of two to four elements, a quarter of them of one, and
		// now and then an empty one.
		const auto draw = extent_distribution(generator);
		const std::size_t extent =
			draw == 0 ? 0 :
			draw < 5 ? 1 :
			2 + draw % 3;
		description.extents.push_back(extent);
	}

	const auto operand_count = operand_distribution(generator);
	for (std::size_t operand = 0; operand < operand_count; ++operand)
	{
		operand_description result;
		if (contiguous(generator))
		{
			result.strides = make_contiguous_strides(
				description.extents,
				generator
			);
		}
		else
		{
			for (std::size_t axis = 0; axis < rank; ++axis)
			{
				result.strides.push_back(stride_distribution(generator));
			}
		}
		result.offset = offset_distribution(generator);
		description.operands.push_back(std::move(result));
	}

	return description;
}

} // anonymous namespace

TEST_CASE(
	"a default constructed joint_layout_builder holds no implementation",
	"[joint_layout_builder]"
)
{
	const joint_layout_builder builder;

	CHECK( builder.get_implementation() == nullptr );
}

TEST_CASE(
	"joint_layout_builder::set_extents sets the extents of the iteration "
	"space",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	joint_layout_builder builder;

	builder.set_extents(make_span(extents));

	const auto *implementation = builder.get_implementation();
	REQUIRE( implementation );
	CHECK( to_vector(implementation->get_extents()) == extents );
	CHECK( implementation->get_number_of_operands() == 0 );
}

TEST_CASE(
	"joint_layout_builder::set_extents throws once the iteration space is "
	"known",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	joint_layout_builder builder;

	SECTION( "from an earlier call" )
	{
		builder.set_extents(make_span(extents));
	}
	SECTION( "from the first operand" )
	{
		builder.add_operand(
			strided_layout::make_contiguous_layout(make_span(extents))
		);
	}

	CHECK_THROWS_MATCHES(
		builder.set_extents(make_span(extents)),
		invalid_operation_error,
		Catch::Matchers::Message(
			"Extents can only be set once and before adding any operand"
		)
	);
}

TEST_CASE(
	"joint_layout_builder::add_operand takes the iteration space from the "
	"first operand when no extents were set",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	joint_layout_builder builder;

	builder.add_operand(
		strided_layout::make_contiguous_layout(make_span(extents))
	);

	const auto *implementation = builder.get_implementation();
	REQUIRE( implementation );
	CHECK( to_vector(implementation->get_extents()) == extents );
	CHECK( implementation->get_number_of_operands() == 1 );
}

TEST_CASE(
	"joint_layout_builder::add_operand stores the strides and the offset of "
	"an operand",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	const std::vector<std::ptrdiff_t> strides = { 1, 20, 120, 1440 };
	const std::ptrdiff_t offset = 7;

	joint_layout_builder builder;
	builder.set_extents(make_span(extents));

	SECTION( "given as a strided layout" )
	{
		builder.add_operand(
			strided_layout::make_custom_layout(
				make_span(extents),
				make_span(strides),
				offset
			)
		);
	}
	SECTION( "given as extents, strides and an offset" )
	{
		builder.add_operand(make_span(extents), make_span(strides), offset);
	}

	const auto *implementation = builder.get_implementation();
	REQUIRE( implementation );
	REQUIRE( implementation->get_number_of_operands() == 1 );
	CHECK( to_vector(implementation->get_strides(0)) == strides );
	CHECK( implementation->get_offset(0) == offset );
}

TEST_CASE(
	"joint_layout_builder::add_operand broadcasts an operand to the "
	"iteration space",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	const bool as_strided_layout = GENERATE(true, false);
	std::vector<std::size_t> operand_extents;
	std::vector<std::ptrdiff_t> operand_strides;
	std::vector<std::ptrdiff_t> expected_strides;

	SECTION( "along an axis of extent one" )
	{
		operand_extents = { 20, 1, 12, 12 };
		operand_strides = { 144, 144, 12, 1 };
		expected_strides = { 144, 0, 12, 1 };
	}
	SECTION( "along the leading axes it does not have" )
	{
		operand_extents = { 12, 12 };
		operand_strides = { 12, 1 };
		expected_strides = { 0, 0, 12, 1 };
	}
	SECTION( "along both" )
	{
		operand_extents = { 1, 12 };
		operand_strides = { 12, 1 };
		expected_strides = { 0, 0, 0, 1 };
	}

	joint_layout_builder builder;
	builder.set_extents(make_span(extents));

	if (as_strided_layout)
	{
		builder.add_operand(
			strided_layout::make_custom_layout(
				make_span(operand_extents),
				make_span(operand_strides)
			)
		);
	}
	else
	{
		builder.add_operand(
			make_span(operand_extents),
			make_span(operand_strides),
			0
		);
	}

	const auto *implementation = builder.get_implementation();
	REQUIRE( implementation );
	CHECK( to_vector(implementation->get_extents()) == extents );
	REQUIRE( implementation->get_number_of_operands() == 1 );
	CHECK( to_vector(implementation->get_strides(0)) == expected_strides );
}

TEST_CASE(
	"joint_layout_builder::add_operand throws on extents it cannot broadcast",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 4, 12, 12 };
	const std::vector<std::size_t> operand_extents = { 20, 6, 12, 12 };

	joint_layout_builder builder;
	builder.set_extents(make_span(extents));

	CHECK_THROWS_MATCHES(
		builder.add_operand(
			strided_layout::make_contiguous_layout(make_span(operand_extents))
		),
		std::invalid_argument,
		Catch::Matchers::Message(
			"Unable to broadcast extent of 6 into target extent of 4."
		)
	);
}

TEST_CASE(
	"joint_layout_builder::build moves the implementation into the layout",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	const auto *implementation = builder.get_implementation();

	const auto layout = builder.build();

	CHECK( builder.get_implementation() == nullptr );
	CHECK( layout.get_implementation() == implementation );
}

TEST_CASE(
	"building without flags keeps the axes as they were added",
	"[joint_layout_builder]"
)
{
	// Row-major, so that reordering and coalescing would both change it.
	const layout_description description = {
		{ 20, 6, 12, 12 },
		{
			{ { 864, 144, 12, 1 }, 0 },
			{ { 864, 144, 12, 1 }, 3 }
		}
	};

	const auto layout = build_layout(description, {});

	CHECK( to_vector(layout.get_extents()) == description.extents );
	CHECK( to_vector(layout.get_strides(0)) ==
	       description.operands[0].strides );
	CHECK( to_vector(layout.get_strides(1)) ==
	       description.operands[1].strides );
	CHECK( layout.get_offset(0) == 0 );
	CHECK( layout.get_offset(1) == 3 );
}

TEST_CASE(
	"reordering walks the first operand from its smallest stride to its "
	"largest, and every other operand along with it",
	"[joint_layout_builder]"
)
{
	const layout_description description = {
		{ 20, 6, 12, 12 },
		{
			{ { 12, 864, 1, 144 }, 0 },
			{ { 1, 20, 120, 1440 }, 0 }
		}
	};

	const auto layout = build_layout(description, reordering_only);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 12, 20, 12, 6 } );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, 12, 144, 864 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 120, 1, 1440, 20 } );
}

TEST_CASE(
	"reordering compares strides too far apart for an int",
	"[joint_layout_builder]"
)
{
	const std::ptrdiff_t far = std::ptrdiff_t(1) << 32;
	const auto strides = GENERATE_COPY(
		std::vector<std::ptrdiff_t>{ 1, far },
		std::vector<std::ptrdiff_t>{ far, 1 }
	);

	const auto layout = build_layout(
		{ { 2, 2 }, { { strides, 0 } } },
		reordering_only
	);

	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, far } );
}

TEST_CASE(
	"reordering walks the shorter of two axes no operand tells apart "
	"innermost, however long they are",
	"[joint_layout_builder]"
)
{
	const std::size_t far = (std::size_t(1) << 32) + 1;
	const auto extents = GENERATE_COPY(
		std::vector<std::size_t>{ 2, far },
		std::vector<std::size_t>{ far, 2 }
	);

	const auto layout = build_layout(
		{ extents, { { { 0, 0 }, 0 } } },
		reordering_only
	);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 2, far } );
}

TEST_CASE(
	"reordering lets a later operand order the axes the first one is "
	"broadcast along",
	"[joint_layout_builder]"
)
{
	// The first operand cannot tell its broadcast axis from the other two, so
	// the second one orders it, and the first one orders the rest.
	const layout_description description = {
		{ 2, 3, 4 },
		{
			{ { 1, 0, 6 }, 0 },
			{ { 1, 2, 6 }, 0 }
		}
	};

	const auto layout = build_layout(description, reordering_only);

	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, 0, 6 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 1, 2, 6 } );
}

TEST_CASE(
	"reordering steps over an axis nothing tells apart from the others",
	"[joint_layout_builder]"
)
{
	// Equal extents and a single operand broadcast along the middle axis,
	// which therefore compares equal to both of the others.
	const layout_description description = {
		{ 2, 2, 2 },
		{ { { 4, 0, 1 }, 0 } }
	};

	const auto layout = build_layout(description, reordering_only);

	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, 0, 4 } );
}

TEST_CASE(
	"reordering follows a reduction output along the axes it tells apart, "
	"and its input along the rest",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> extents = { 20, 6, 12, 12 };
	const std::vector<std::ptrdiff_t> input_strides = { 3456, 288, 12, 1 };
	const auto output_strides = GENERATE(
		std::vector<std::ptrdiff_t>{ 12, 0, 1, 0 },
		std::vector<std::ptrdiff_t>{ 0, 12, 0, 1 },
		std::vector<std::ptrdiff_t>{ 12, 0, 0, 1 },
		std::vector<std::ptrdiff_t>{ 0, 0, 24, 1 }
	);

	const auto layout = build_layout(
		{ extents, { { output_strides, 0 }, { input_strides, 0 } } },
		reordering_and_coalescing
	);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>(extents.crbegin(), extents.crend()) );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>(
	           output_strides.crbegin(),
	           output_strides.crend()
	       ) );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>(
	           input_strides.crbegin(),
	           input_strides.crend()
	       ) );
}

TEST_CASE(
	"coalescing merges axes every operand walks contiguously",
	"[joint_layout_builder]"
)
{
	const layout_description description = {
		{ 20, 6, 12, 12 },
		{
			{ { 1, 20, 120, 1440 }, 0 },
			{ { 1, 20, 120, 1440 }, 0 }
		}
	};

	const auto layout = build_layout(description, coalescing_only);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 17280 } );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 1 } );
}

TEST_CASE(
	"coalescing keeps apart axes some operand does not walk contiguously",
	"[joint_layout_builder]"
)
{
	// The first operand skips along its second axis, the second operand along
	// its last.
	const layout_description description = {
		{ 20, 6, 12, 12 },
		{
			{ { 1, 40, 240, 2880 }, 0 },
			{ { 1, 20, 120, 2880 }, 0 }
		}
	};

	const auto layout = build_layout(description, coalescing_only);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 20, 72, 12 } );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, 40, 2880 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 1, 20, 2880 } );
}

TEST_CASE(
	"coalescing drops the axes of extent one",
	"[joint_layout_builder]"
)
{
	const layout_description description = {
		{ 20, 1, 1, 12 },
		{
			{ { 1, 2, 4, 20 }, 0 },
			{ { 1, 2, 4, 20 }, 0 }
		}
	};

	const auto layout = build_layout(description, coalescing_only);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 240 } );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 1 } );
}

TEST_CASE(
	"coalescing leaves a single axis of extent one when every axis has "
	"extent one",
	"[joint_layout_builder]"
)
{
	const layout_description description = {
		{ 1, 1, 1 },
		{ { { 5, 7, 9 }, 3 } }
	};

	const auto layout = build_layout(description, coalescing_only);

	CHECK( to_vector(layout.get_extents()) == std::vector<std::size_t>{ 1 } );
	CHECK( layout.get_offset(0) == 3 );
}

TEST_CASE(
	"reordering and coalescing merge what becomes contiguous once the axes "
	"are sorted",
	"[joint_layout_builder]"
)
{
	// Row-major, skipping every other element along the second axis.
	const layout_description description = {
		{ 20, 6, 12, 12 },
		{
			{ { 1728, 288, 12, 1 }, 0 },
			{ { 1728, 288, 12, 1 }, 0 }
		}
	};

	const auto layout = build_layout(description, reordering_and_coalescing);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 144, 120 } );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, 288 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 1, 288 } );
}

TEST_CASE(
	"reordering and coalescing merge the axes a reduction keeps apart from "
	"the axes it folds",
	"[joint_layout_builder]"
)
{
	const layout_description description = {
		{ 20, 6, 12, 12 },
		{
			{ { 0, 0, 12, 1 }, 0 },
			{ { 864, 144, 12, 1 }, 0 }
		}
	};

	const auto layout = build_layout(description, reordering_and_coalescing);

	CHECK( to_vector(layout.get_extents()) ==
	       std::vector<std::size_t>{ 144, 120 } );
	CHECK( to_vector(layout.get_strides(0)) ==
	       std::vector<std::ptrdiff_t>{ 1, 0 } );
	CHECK( to_vector(layout.get_strides(1)) ==
	       std::vector<std::ptrdiff_t>{ 1, 144 } );
}

TEST_CASE(
	"reordering and coalescing walk a contiguous array as a single axis "
	"whatever order its axes are laid out in",
	"[joint_layout_builder]"
)
{
	const std::vector<std::size_t> array_extents = { 2, 3, 4, 5 };
	const std::vector<std::ptrdiff_t> row_major_strides = { 60, 20, 5, 1 };
	const std::ptrdiff_t sign = GENERATE(1, -1);

	// Axis `a` of the layout is axis order[a] of the array, so every
	// permutation of the order lays the same array out differently.
	std::vector<std::size_t> order = { 0, 1, 2, 3 };
	do
	{
		layout_description description;
		description.operands.push_back({ {}, 0 });
		for (const auto array_axis : order)
		{
			description.extents.push_back(array_extents[array_axis]);
			description.operands[0].strides.push_back(
				sign*row_major_strides[array_axis]
			);
		}

		INFO(
			"axis order " << order[0] << order[1] << order[2] << order[3] <<
			", sign " << sign
		);

		const auto reordered = build_layout(description, reordering_only);
		CHECK( to_vector(reordered.get_extents()) ==
		       std::vector<std::size_t>{ 5, 4, 3, 2 } );
		CHECK( to_vector(reordered.get_strides(0)) ==
		       std::vector<std::ptrdiff_t>{ sign, 5*sign, 20*sign, 60*sign } );

		const auto coalesced =
			build_layout(description, reordering_and_coalescing);
		CHECK( to_vector(coalesced.get_extents()) ==
		       std::vector<std::size_t>{ 120 } );
		CHECK( to_vector(coalesced.get_strides(0)) ==
		       std::vector<std::ptrdiff_t>{ sign } );
	}
	while (std::next_permutation(order.begin(), order.end()));
}

TEST_CASE(
	"a layout built with any flags visits every combination of offsets the "
	"operands hold, and nothing else",
	"[joint_layout_builder]"
)
{
	const auto seed = GENERATE(range(0u, 256u));
	const auto description = make_random_description(seed);
	const auto expected = enumerate_offsets(description);

	const std::vector<joint_layout_build_flags> flag_combinations = {
		{},
		reordering_only,
		coalescing_only,
		reordering_and_coalescing
	};

	for (std::size_t i = 0; i < flag_combinations.size(); ++i)
	{
		INFO( "seed " << seed << ", flag combination " << i );

		const auto layout = build_layout(description, flag_combinations[i]);
		CHECK( layout.compute_element_count() == expected.size() );
		CHECK( visit_offsets(layout) == expected );
	}
}

TEST_CASE(
	"reordering only permutes the axes, each keeping its extent and the "
	"stride of every operand along it",
	"[joint_layout_builder]"
)
{
	const auto seed = GENERATE(range(0u, 256u));
	const auto description = make_random_description(seed);
	INFO( "seed " << seed );

	const auto verbatim = build_layout(description, {});
	const auto reordered = build_layout(description, reordering_only);

	CHECK( axis_columns(reordered) == axis_columns(verbatim) );
}

TEST_CASE(
	"reordering walks a first operand with no broadcast axis from its "
	"smallest stride to its largest",
	"[joint_layout_builder]"
)
{
	const auto seed = GENERATE(range(0u, 256u));
	auto description = make_random_description(seed);
	for (auto &stride : description.operands[0].strides)
	{
		if (stride == 0)
		{
			stride = 1;
		}
	}
	INFO( "seed " << seed );

	const auto reordered = build_layout(description, reordering_only);

	const auto strides = reordered.get_strides(0);
	for (std::size_t axis = 1; axis < strides.size(); ++axis)
	{
		INFO( "axis " << axis );
		CHECK( std::abs(strides[axis - 1]) <= std::abs(strides[axis]) );
	}
}

TEST_CASE(
	"coalescing leaves no axis of extent one and no two adjacent axes every "
	"operand could walk as one",
	"[joint_layout_builder]"
)
{
	const auto seed = GENERATE(range(0u, 256u));
	const auto description = make_random_description(seed);
	INFO( "seed " << seed );

	const auto coalesced = build_layout(description, coalescing_only);
	const auto extents = coalesced.get_extents();

	if (extents.size() > 1)
	{
		for (const auto extent : extents)
		{
			CHECK( extent != 1 );
		}
	}

	for (std::size_t axis = 1; axis < extents.size(); ++axis)
	{
		INFO( "axes " << axis - 1 << " and " << axis );

		bool mergeable = true;
		for (
			std::size_t operand = 0;
			operand < coalesced.get_number_of_operands();
			++operand
		)
		{
			const auto strides = coalesced.get_strides(operand);
			const auto walked =
				static_cast<std::ptrdiff_t>(extents[axis - 1]) *
				strides[axis - 1];
			mergeable = mergeable && walked == strides[axis];
		}
		CHECK( !mergeable );
	}
}
