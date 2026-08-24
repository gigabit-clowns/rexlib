// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/dispatch/program_cache.hpp>

#include <memory>

namespace rexlib
{

class program_manager;

/**
 * @brief Eager implementation of @ref dispatcher.
 *
 * Each call to @ref dispatch immediately builds the executable program for the
 * requested operation and submits it to the provided queue. Backend-private
 * resources produced while building programs (compiled kernels, FFT plans,
 * ...) are kept in an internal @ref program_cache so that they are
 * reused across builds with matching launch configurations.
 */
class eager_dispatcher final
	: public dispatcher
{
public:
	/**
	 * @brief Construct an eager dispatcher.
	 *
	 * @param program_manager The manager queried to build the program for each
	 * operation. Must not be null.
	 * @param cache_capacity Maximum number of backend-private resources kept in
	 * the internal cache.
	 * @throws std::invalid_argument When @p program_manager is null.
	 */
	eager_dispatcher(
		std::shared_ptr<const program_manager> program_manager,
		std::size_t cache_capacity
	);
	~eager_dispatcher() override;

	void dispatch(
		const operation &operation,
		span<array> output_operands,
		span<const const_array_ref> input_operands,
		const device_context &device_context
	) override;

private:
	std::shared_ptr<const program_manager> m_program_manager;
	program_cache m_program_cache;
};

} // namespace rexlib
