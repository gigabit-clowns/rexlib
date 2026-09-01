// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/view.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>

#include <core/ndarray/array_implementation.hpp>

#include <memory>
#include <stdexcept>

namespace rexlib
{

static std::shared_ptr<const array_implementation> make_subarray_implementation(
	const array_implementation *parent,
	span<const dynamic_subscript> subscripts
)
{
	if (!parent)
	{
		throw std::invalid_argument(
			"subarray: The input array is not initialized."
		);
	}

	const auto &descriptor = parent->get_descriptor();
	return std::make_shared<const array_implementation>(
		parent->share_storage(),
		array_descriptor(
			descriptor.get_layout().apply_subscripts(subscripts),
			descriptor.get_data_type()
		)
	);
}

array subarray(array &input, span<const dynamic_subscript> subscripts)
{
	return array(
		make_subarray_implementation(input.get_implementation(), subscripts)
	);
}

const_array subarray(
	const_array_ref input,
	span<const dynamic_subscript> subscripts
)
{
	return const_array(
		make_subarray_implementation(input.get_implementation(), subscripts)
	);
}

} // namespace rexlib
