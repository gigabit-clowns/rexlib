// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_read_format_manager;

/**
 * @brief A provider that opens a file every time it is asked for one.
 *
 * It keeps nothing: every reader it returns is newly opened through the
 * format manager it was constructed with, so a path asked for twice is
 * opened twice and the two readers are unrelated. With no capacity to size
 * and no eviction, a file is opened exactly as often as it is asked for.
 */
class REXLIB_API direct_image_reader_provider final
	: public image_reader_provider
{
public:
	/**
	 * @brief Construct a provider opening through a format manager.
	 *
	 * @param formats The formats a file may be opened with.
	 * @throws std::invalid_argument If @p formats is null.
	 */
	explicit direct_image_reader_provider(
		std::shared_ptr<const image_read_format_manager> formats
	);

	~direct_image_reader_provider() override;

	std::shared_ptr<const image_reader>
	acquire(const std::string &path) override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::shared_ptr<const image_read_format_manager> m_formats;
};

} // namespace em
} // namespace rexlib
