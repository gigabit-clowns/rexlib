// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpu_execution_context_fixture.hpp"
#include "verb_type_sweep.hpp"

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/creation.hpp>

#include <xmipp4/core/hardware/memory_resource_affinity.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/numerical/numerical_type_traits.hpp>

#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

namespace rex
{
namespace test
{

/**
 * @brief Fixture driving the Fourier transform verbs.
 *
 * A transform is not a fold over a domain the way the elementwise and
 * reduction families are, so there is no rule driven sweep here: what a
 * transform computes is one thing, stated once, and the types it is stated at
 * are chosen by the case. The precision is what the tolerance is for.
 */
class fourier_verb_fixture : public cpu_execution_context_fixture
{
protected:
	/**
	 * @brief Build an operand holding a given sequence of values.
	 *
	 * @tparam T The element type.
	 * @param extents The extents of the operand.
	 * @param values One value per element, in memory order. Each carries an
	 * imaginary part, so one list serves a real and a complex operand alike.
	 * @return array The operand.
	 */
	template <typename T>
	array make_operand(
		std::vector<std::size_t> extents,
		const std::vector<element_value> &values
	) const
	{
		auto result = empty(
			make_descriptor(std::move(extents), numerical_type_of<T>::value),
			memory_resource_affinity::device,
			context
		);

		auto *storage = result.get_storage();
		REQUIRE( storage != nullptr );
		auto *data = static_cast<T*>(storage->get_host_ptr());
		REQUIRE( data != nullptr );

		for (std::size_t i = 0; i < values.size(); ++i)
		{
			data[i] = values[i].as<T>();
		}

		return result;
	}

	/**
	 * @brief Check an output element by element.
	 *
	 * A transform is a sum of as many rounded products as it has samples, so
	 * the comparison is a tolerance rather than an equality. The tolerance is
	 * absolute, the cases below being written at magnitudes of the order of
	 * their own extents.
	 *
	 * @tparam U The element type the output is expected to have.
	 * @param result The output array.
	 * @param extents The extents the output is expected to have.
	 * @param expected One value per element, in memory order.
	 * @param tolerance How far each element may be from its expected value.
	 */
	template <typename U>
	void check_values(
		const array &result,
		const std::vector<std::size_t> &extents,
		const std::vector<element_value> &expected,
		double tolerance = 1e-4
	) const
	{
		const auto data_type = numerical_type_of<U>::value;
		CHECK( result.get_descriptor() == make_descriptor(extents, data_type) );

		const auto values = read_host<U>(result, expected.size());
		for (std::size_t i = 0; i < expected.size(); ++i)
		{
			INFO( "element " << i );
			const auto deviation = std::abs(values[i] - expected[i].as<U>());
			CHECK( static_cast<double>(deviation) <= tolerance );
		}
	}
};

} // namespace test
} // namespace rex
