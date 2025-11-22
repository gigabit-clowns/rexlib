// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/plugin.hpp>

namespace xmipp4 
{

class cuda_plugin final
	: public plugin
{
public:
	cuda_plugin() = default;
	cuda_plugin(const cuda_plugin& other) = default;
	cuda_plugin(cuda_plugin&& other) = default;
	~cuda_plugin() override = default;

	cuda_plugin& operator=(const cuda_plugin& other) = default;
	cuda_plugin& operator=(cuda_plugin&& other) = default;

	const std::string& get_name() const noexcept override;
	version get_version() const noexcept override;
	void register_at(service_catalog& catalog) const override;

private:
	static const std::string name;

};

} // namespace xmipp4
