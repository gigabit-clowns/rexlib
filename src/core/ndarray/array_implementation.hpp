// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include <memory>

namespace rexlib 
{

class array_implementation
{
public:
	array_implementation() = default;
	array_implementation(
		std::shared_ptr<buffer> storage,
		array_descriptor descriptor
	) noexcept;
	array_implementation(const array_implementation &other) = default;
	array_implementation(array_implementation &&other) = default;
	~array_implementation() = default;

	array_implementation&
	operator=(const array_implementation &other) = default;
	array_implementation&
	operator=(array_implementation &&other) = default;

	const array_descriptor& get_descriptor() const noexcept;

	buffer* get_storage() const noexcept;

	const std::shared_ptr<buffer>& share_storage() const noexcept;

private:
	std::shared_ptr<buffer> m_storage;
	array_descriptor m_descriptor;

};

} // namespace rexlib

#include "array_implementation.inl"
