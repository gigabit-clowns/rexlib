// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/dispatch/program_manager.hpp>

#include <xmipp4/core/exceptions/invalid_operation_error.hpp>
#include <xmipp4/core/dispatch/program_builder.hpp>
#include <xmipp4/core/dispatch/operation.hpp>
#include <xmipp4/core/dispatch/operation_id.hpp>

#include <core/find_most_suitable_backend.hpp>
#include <core/dispatch/core_program_builder_registry.hpp>

#include <vector>
#include <unordered_map>

namespace xmipp4
{

class program_manager::implementation
{
public:
	bool register_builder(
		std::unique_ptr<program_builder> builder
	)
	{
		REX_ASSERT(builder);
		const auto &op_id = builder->get_operation_id();
		m_builders[op_id].push_back(std::move(builder));
		return true;
	}

	program_builder* get_most_suitable_builder(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
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
			[&operation, &output_signatures, &input_signatures, &queue]
			(const auto &item)
			{
				return item->get_suitability(
					operation,
					output_signatures,
					input_signatures,
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
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		command_queue &queue,
		program_cache *cache
	) const
	{
		const auto *builder = get_most_suitable_builder(
			operation,
			output_signatures,
			input_signatures,
			queue
		);

		if (!builder)
		{
			throw invalid_operation_error(
				"Could not find a suitable operation program builder for the "
				"requested operation"
			);
		}

		return builder->build(
			operation,
			output_signatures,
			input_signatures,
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

program_manager::program_manager() noexcept = default;
program_manager::~program_manager() = default;

void program_manager::register_builtin_backends()
{
	get_core_program_builder_registry().register_all(*this);
}

bool program_manager::register_builder(
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
program_manager::build(
	const operation &operation,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	command_queue &queue,
	program_cache *cache
) const
{
	return get_implementation().build(
		operation,
		output_signatures,
		input_signatures,
		queue,
		cache
	);
}

program_manager::implementation&
program_manager::create_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}

	return *m_implementation;
}

const program_manager::implementation&
program_manager::get_implementation() const noexcept
{
	static const implementation empty_implementation;
	return m_implementation ? *m_implementation : empty_implementation;
}

} // namespace xmipp4
