// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/plugin.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The CPU backend, as a plugin built into the library.
 *
 * Registers the CPU device backend and the CPU's program builders at a
 * catalog. Its version is the library's.
 */
class cpu_plugin final
	: public plugin
{
public:
	cpu_plugin() = default;
	cpu_plugin(const cpu_plugin &other) = default;
	cpu_plugin(cpu_plugin &&other) = default;
	~cpu_plugin() override = default;

	cpu_plugin& operator=(const cpu_plugin &other) = default;
	cpu_plugin& operator=(cpu_plugin &&other) = default;

	const std::string& get_name() const noexcept override;
	version get_version() const noexcept override;
	void register_at(service_catalog &catalog) const override;
};

} // namespace cpu
} // namespace rexlib
