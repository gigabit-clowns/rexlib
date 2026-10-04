// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include "../mock/mock_image_read_format.hpp"
#include "../mock/mock_image_write_format.hpp"

#include <memory>
#include <trompeloeil.hpp>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * A format manager whose formats are mocks, each reporting a fixed
 * suitability for every file.
 *
 * The manager owns the formats it is given, so the expectations on them have
 * to be destroyed first. This holds both, in that order, and shares the
 * manager with whatever else a test hands it to.
 */
template <typename Manager, typename Format>
class format_manager_fixture
{
public:
	format_manager_fixture()
		: m_manager(std::make_shared<Manager>())
	{
	}

	/**
	 * Register a mock format reporting @p suitability for every file.
	 *
	 * Further expectations may be set on the format returned, provided they
	 * are destroyed before this fixture.
	 */
	Format& add_format(backend_priority suitability)
	{
		auto format = std::make_unique<Format>();
		auto &result = *format;
		m_expectations.push_back(
			NAMED_ALLOW_CALL(result, get_suitability(ANY(const image_probe&)))
				.RETURN(suitability)
		);
		m_manager->register_format(std::move(format));
		return result;
	}

	const std::shared_ptr<Manager>& get_manager() const noexcept
	{
		return m_manager;
	}

private:
	std::shared_ptr<Manager> m_manager;
	std::vector<std::unique_ptr<trompeloeil::expectation>> m_expectations;
};

using read_format_manager_fixture = format_manager_fixture<
	image_read_format_manager,
	mock_image_read_format
>;

using write_format_manager_fixture = format_manager_fixture<
	image_write_format_manager,
	mock_image_write_format
>;

} // namespace em
} // namespace rexlib
