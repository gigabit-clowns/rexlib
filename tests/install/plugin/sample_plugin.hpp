// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/plugin.hpp>

#include <string>

namespace sample
{

/**
 * @brief A plugin that adds the sample device and the programs it runs.
 */
class sample_plugin final
	: public rexlib::plugin
{
public:
	const std::string& get_name() const noexcept override;
	rexlib::version get_version() const noexcept override;
	void register_at(rexlib::service_catalog &catalog) const override;
};

} // namespace sample
