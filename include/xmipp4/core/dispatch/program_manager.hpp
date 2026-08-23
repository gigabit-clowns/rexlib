// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"
#include "../service_manager.hpp"
#include "../numerical/numerical_type.hpp"
#include "../span.hpp"

#include <memory>

namespace xmipp4
{

class program;
class command_queue;


class operation;
class operand_signature;
class program_builder;
class program_cache;

/**
 * @brief Centralizes all known instances of operation program builders.
 *
 * The operation program builders are cataloged by their intended operation such
 * that it allows fast retrieval.
 */
class REXLIB_API program_manager
	: public service_manager
{
public:
	program_manager() noexcept;
	program_manager(const program_manager &other) = delete;
	program_manager(program_manager &&other) = delete;
	~program_manager() override;

	program_manager&
	operator=(const program_manager &other) = delete;
	program_manager&
	operator=(program_manager &&other) = delete;

	void register_builtin_backends() override;

	/**
	 * @brief Register a new operation program builder.
	 *
	 * @param builder The builder to be registered.
	 * @return true The operation program builder was successfully registered.
	 * @return false The operation program builder could not be registered.
	 */
	bool register_builder(
		std::unique_ptr<program_builder> builder
	);

	/**
	 * @brief Build an executable operation program for a given launch
	 * configuration.
	 *
	 * @param operation The operation for which the program is built.
	 * @param output_signatures The output array signatures involved in the
	 * operation.
	 * @param input_signatures The input array signatures involved in the
	 * operation.
	 * @param queue The command queue where the built program is
	 * intended to be used.
	 * @param cache Optional cache for backend-private resources, forwarded
	 * to the selected builder.
	 * @return std::shared_ptr<program> The executable program suited
	 * for the requested operation and signature.
	 */
	std::shared_ptr<program> build(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		command_queue &queue,
		program_cache *cache = nullptr
	) const;

private:
	class implementation;
	XMIPP4_STD_MEMBER_INTERFACE 
	std::unique_ptr<implementation> m_implementation;

	implementation& create_if_null();
	const implementation& get_implementation() const noexcept;
};

} // namespace xmipp4
