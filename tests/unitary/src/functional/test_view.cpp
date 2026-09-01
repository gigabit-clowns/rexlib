// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/view.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/layout/slice.hpp>
#include <rexlib/core/layout/subscript_tags.hpp>

#include "../core/hardware/mock/mock_buffer.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;

class subarray_fixture
{
public:
	subarray_fixture()
		: m_storage(std::make_shared<mock_buffer>())
	{
		const std::vector<std::size_t> extents = {4, 6, 8};
		m_parent = array(
			m_storage,
			array_descriptor(
				strided_layout::make_contiguous_layout(make_span(extents)),
				numerical_type::float32
			)
		);
	}

	array& get_parent() noexcept
	{
		return m_parent;
	}

	const std::shared_ptr<mock_buffer>& get_storage() const noexcept
	{
		return m_storage;
	}

private:
	std::shared_ptr<mock_buffer> m_storage;
	array m_parent;
};

static std::vector<std::size_t> extents_of(const array_descriptor &descriptor)
{
	std::vector<std::size_t> extents;
	descriptor.get_layout().get_extents(extents);
	return extents;
}

static std::vector<std::ptrdiff_t>
strides_of(const array_descriptor &descriptor)
{
	std::vector<std::ptrdiff_t> strides;
	descriptor.get_layout().get_strides(strides);
	return strides;
}

TEST_CASE( "subarray selects a part of an array", "[view]" )
{
	subarray_fixture fixture;

	SECTION( "an index drops the axis it addresses" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			std::ptrdiff_t(2), all(), all()
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{6, 8} );
		REQUIRE( view.get_descriptor().get_layout().get_offset() == 2 * 6 * 8 );
	}

	SECTION( "a slice narrows the axis it addresses" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			all(), make_slice(1, 3), all()
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{4, 3, 8} );
		REQUIRE( view.get_descriptor().get_layout().get_offset() == 8 );
	}

	SECTION( "a step makes the view non contiguous" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			all(), all(), make_slice(0, 4, 2)
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{4, 6, 4} );
		REQUIRE( strides_of(view.get_descriptor()).back() == 2 );
	}

	SECTION( "an ellipsis keeps the axes it stands for" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			std::ptrdiff_t(1), ellipsis()
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{6, 8} );
	}

	SECTION( "a new axis introduces an extent of one" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			new_axis(), ellipsis()
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{1, 4, 6, 8} );
	}

	SECTION( "a batch slot keeps its rank with a leading extent of one" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			make_slice(2, 1), all(), all()
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{1, 6, 8} );
		REQUIRE( view.get_descriptor().get_layout().get_offset() == 2 * 6 * 8 );
	}
}

TEST_CASE( "subarray shares the storage of its input", "[view]" )
{
	subarray_fixture fixture;
	const std::vector<dynamic_subscript> subscripts = {
		std::ptrdiff_t(1), all(), all()
	};

	SECTION( "the view points at the same buffer" )
	{
		auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( view.get_storage() == fixture.get_storage().get() );
		REQUIRE( view.share_storage() == fixture.get_storage() );
	}

	SECTION( "the view keeps the buffer alive on its own" )
	{
		auto view = subarray(fixture.get_parent(), make_span(subscripts));
		fixture.get_parent() = array();

		REQUIRE( view.get_storage() == fixture.get_storage().get() );
	}

	SECTION( "a view of a view composes" )
	{
		auto view = subarray(fixture.get_parent(), make_span(subscripts));
		const std::vector<dynamic_subscript> inner = {
			std::ptrdiff_t(2), all()
		};
		const auto nested = subarray(view, make_span(inner));

		REQUIRE( extents_of(nested.get_descriptor()) ==
			std::vector<std::size_t>{8} );
		REQUIRE( nested.get_descriptor().get_layout().get_offset() ==
			(1 * 6 * 8) + (2 * 8) );
	}

	SECTION( "the data type is carried over unchanged" )
	{
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( view.get_descriptor().get_data_type() ==
			numerical_type::float32 );
	}
}

TEST_CASE( "subarray has a read-only overload", "[view]" )
{
	subarray_fixture fixture;
	const std::vector<dynamic_subscript> subscripts = {
		std::ptrdiff_t(1), all(), all()
	};

	SECTION( "a const_array_ref yields a const_array" )
	{
		const const_array_ref input(fixture.get_parent());
		const auto view = subarray(input, make_span(subscripts));

		REQUIRE( view.get_storage() == fixture.get_storage().get() );
		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{6, 8} );
	}

	SECTION( "a const array selects the read-only overload" )
	{
		const array &input = fixture.get_parent();
		const auto view = subarray(input, make_span(subscripts));

		REQUIRE( view.share_storage() == fixture.get_storage() );
	}
}

TEST_CASE( "subarray rejects what it can not view", "[view]" )
{
	subarray_fixture fixture;

	SECTION( "an uninitialized input is rejected" )
	{
		array empty;
		const std::vector<dynamic_subscript> subscripts = { ellipsis() };

		REQUIRE_THROWS_AS(
			subarray(empty, make_span(subscripts)),
			std::invalid_argument
		);
	}

	SECTION( "an out of bounds index is rejected" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			std::ptrdiff_t(4), all(), all()
		};

		REQUIRE_THROWS_AS(
			subarray(fixture.get_parent(), make_span(subscripts)),
			std::out_of_range
		);
	}

	SECTION( "more subscripts than axes is rejected" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			all(), all(), all(), all()
		};

		REQUIRE_THROWS_AS(
			subarray(fixture.get_parent(), make_span(subscripts)),
			std::invalid_argument
		);
	}
}

TEST_CASE( "subarray keeps the axes it is not given", "[view]" )
{
	subarray_fixture fixture;

	SECTION( "trailing axes are kept whole" )
	{
		const std::vector<dynamic_subscript> subscripts = {
			std::ptrdiff_t(1)
		};
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{6, 8} );
		REQUIRE( view.get_descriptor().get_layout().get_offset() == 6 * 8 );
	}

	SECTION( "an empty subscript list views the whole array" )
	{
		const std::vector<dynamic_subscript> subscripts;
		const auto view = subarray(fixture.get_parent(), make_span(subscripts));

		REQUIRE( extents_of(view.get_descriptor()) ==
			std::vector<std::size_t>{4, 6, 8} );
		REQUIRE( view.get_descriptor().get_layout().get_offset() == 0 );
	}
}
