// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/program_service.hpp>

#include <rexlib/core/dispatch/program_builder.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/operation/operation.hpp>
#include <rexlib/core/operation/operation_id.hpp>

#include <core/find_most_suitable_backend.hpp>

#include <vector>
#include <unordered_map>

namespace rexlib
{

class program_service::implementation
{
public:
	bool register_builder(
		std::unique_ptr<program_builder> builder
	)
	{
		REXLIB_ASSERT(builder);
		const auto &op_id = builder->get_operation_id();
		m_builders[op_id].push_back(std::move(builder));
		return true;
	}

	program_builder* get_most_suitable_builder(
		const operation &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		command_queue &queue
	) const
	{
		const auto op_id = operation.get_id();
		const auto ite1 = m_builders.find(op_id);
		if (ite1 == m_builders.end())
		{
			return nullptr;
		}

		const auto &available_backends = ite1->second;
		const auto ite2 = find_most_suitable_backend(
			available_backends.begin(),
			available_backends.end(),
			[&operation, &output_descriptors, &input_descriptors, &queue]
			(const auto &item)
			{
				return item->get_suitability(
					operation,
					output_descriptors,
					input_descriptors,
					queue
				);
			}
		);

		if (ite2 == available_backends.cend())
		{
			return nullptr;
		}

		return ite2->get();
	}

	std::shared_ptr<program> build(
		const operation &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		command_queue &queue,
		program_cache *cache
	) const
	{
		const auto *builder = get_most_suitable_builder(
			operation,
			output_descriptors,
			input_descriptors,
			queue
		);

		if (!builder)
		{
			throw unsupported_operation_error(
				"Could not find a suitable operation program builder for the "
				"requested operation"
			);
		}

		return builder->build(
			operation,
			output_descriptors,
			input_descriptors,
			queue,
			cache
		);
	}

private:
	std::unordered_map<
		operation_id,
		std::vector<std::unique_ptr<program_builder>>
	> m_builders;

};

program_service::program_service() noexcept = default;
program_service::~program_service() = default;

bool program_service::register_builder(
	std::unique_ptr<program_builder> builder
)
{
	if (!builder)
	{
		return false;
	}

	return create_if_null().register_builder(std::move(builder));
}

std::shared_ptr<program>
program_service::build(
	const operation &operation,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	command_queue &queue,
	program_cache *cache
) const
{
	return get_implementation().build(
		operation,
		output_descriptors,
		input_descriptors,
		queue,
		cache
	);
}

program_service::implementation&
program_service::create_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}

	return *m_implementation;
}

const program_service::implementation&
program_service::get_implementation() const noexcept
{
	static const implementation empty_implementation;
	return m_implementation ? *m_implementation : empty_implementation;
}

} // namespace rexlib
