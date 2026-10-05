// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/scratch_image_reader_provider.hpp>

#include "scratch_image_reader.hpp"

#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_scratch.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

scratch_image_reader_provider::scratch_image_reader_provider(
	std::shared_ptr<image_reader_provider> backing,
	std::shared_ptr<image_scratch> scratch
)
	: m_backing(std::move(backing))
	, m_scratch(std::move(scratch))
{
	if (!m_backing)
	{
		throw std::invalid_argument(
			"scratch_image_reader_provider: The backing provider must not "
			"be null."
		);
	}

	if (!m_scratch)
	{
		throw std::invalid_argument(
			"scratch_image_reader_provider: The scratch must not be null."
		);
	}
}

scratch_image_reader_provider::~scratch_image_reader_provider() = default;

std::shared_ptr<const image_reader>
scratch_image_reader_provider::acquire(const std::string &path)
{
	auto file = m_backing->acquire(path);
	REXLIB_ASSERT(file);

	auto entry = m_scratch->find(path);
	if (!entry)
	{
		return file;
	}

	return std::make_shared<scratch_image_reader>(
		std::move(entry),
		std::move(file)
	);
}

} // namespace em
} // namespace rexlib
