// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_read_format_manager.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <core/find_most_suitable_backend.hpp>
#include <em/image/core_image_format_registry.hpp>

#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

class image_read_format_manager::implementation
{
public:
	bool register_format(std::unique_ptr<image_read_format> format)
	{
		REXLIB_ASSERT(format);
		m_formats.push_back(std::move(format));
		return true;
	}

	const image_read_format*
	get_most_suitable_format(const image_probe &probe) const
	{
		const auto ite = find_most_suitable_backend(
			m_formats.begin(),
			m_formats.end(),
			[&probe] (const auto &item)
			{
				return item->get_suitability(probe);
			}
		);

		if (ite == m_formats.cend())
		{
			return nullptr;
		}

		return ite->get();
	}

	std::unique_ptr<image_reader> open(const image_probe &probe) const
	{
		const auto *format = get_most_suitable_format(probe);
		if (!format)
		{
			throw invalid_operation_error(
				"Could not find a suitable image format to read the "
				"requested file"
			);
		}

		return format->open(probe);
	}

private:
	std::vector<std::unique_ptr<image_read_format>> m_formats;

};

image_read_format_manager::image_read_format_manager() noexcept = default;
image_read_format_manager::~image_read_format_manager() = default;

void image_read_format_manager::register_builtin_backends()
{
	get_core_image_read_format_registry().register_all(*this);
}

bool image_read_format_manager::register_format(
	std::unique_ptr<image_read_format> format
)
{
	if (!format)
	{
		return false;
	}

	return create_if_null().register_format(std::move(format));
}

std::unique_ptr<image_reader>
image_read_format_manager::open(const std::string &path) const
{
	return get_implementation().open(image_probe(path));
}

const image_read_format*
image_read_format_manager::get_most_suitable_format(
	const image_probe &probe
) const
{
	return get_implementation().get_most_suitable_format(probe);
}

image_read_format_manager::implementation&
image_read_format_manager::create_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}

	return *m_implementation;
}

const image_read_format_manager::implementation&
image_read_format_manager::get_implementation() const noexcept
{
	static const implementation empty_implementation;
	return m_implementation ? *m_implementation : empty_implementation;
}

} // namespace em
} // namespace rexlib
