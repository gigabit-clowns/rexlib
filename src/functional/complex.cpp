// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/complex.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/ops/complex/angle_operation.hpp>
#include <rexlib/ops/complex/conjugate_operation.hpp>

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace rexlib
{

namespace
{

bool is_complex(const array_descriptor &descriptor) noexcept
{
	return get_category(descriptor.get_data_type()) ==
		numerical_type_category::complex;
}

/**
 * @brief Describe one of the two real parts interleaved in a complex array.
 *
 * std::complex is layout compatible with an array of two of its value type,
 * so a complex array is a real array of twice as many elements with the two
 * parts alternating. Doubling the strides steps over one whole complex
 * element, and the offset picks which of the two parts is landed on.
 *
 * @param source The descriptor of the complex array.
 * @param part The part to describe: 0 for the real one, 1 for the
 * imaginary one.
 * @return array_descriptor The descriptor of the view.
 */
array_descriptor make_part_descriptor(
	const array_descriptor &source,
	std::ptrdiff_t part
)
{
	const auto &layout = source.get_layout();

	std::vector<std::size_t> extents;
	layout.get_extents(extents);

	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);
	for (auto &stride : strides)
	{
		stride *= 2;
	}

	return array_descriptor(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			2*layout.get_offset() + part
		),
		make_real(source.get_data_type())
	);
}

} // anonymous namespace

array real(array &x)
{
	const auto &descriptor = x.get_descriptor();
	if (!is_complex(descriptor))
	{
		// A real array is already its own real part.
		return x.share();
	}

	return array(x.share_storage(), make_part_descriptor(descriptor, 0));
}

array imag(array &x)
{
	const auto &descriptor = x.get_descriptor();
	if (!is_complex(descriptor))
	{
		std::ostringstream oss;
		oss << "imag: the array has data type "
			<< descriptor.get_data_type()
			<< ", whose imaginary part is zero everywhere and therefore "
			<< "not reachable as a view of its storage. Build it with "
			<< "zeros() instead.";
		throw std::invalid_argument(oss.str());
	}

	return array(x.share_storage(), make_part_descriptor(descriptor, 1));
}

array conjugate(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::conjugate_operation(), x, context, out);
}

array angle(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::angle_operation(), x, context, out);
}

} // namespace rexlib
