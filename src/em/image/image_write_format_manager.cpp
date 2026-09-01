// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_write_format_manager.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_open_options.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <core/find_most_suitable_backend.hpp>
#include <em/image/core_image_format_registry.hpp>

#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

class image_write_format_manager::implementation
{
public:
	bool register_format(std::unique_ptr<image_write_format> format)
	{
		REXLIB_ASSERT(format);
		m_formats.push_back(std::move(format));
		return true;
	}

	const image_write_format*
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

	std::unique_ptr<image_writer> open(
		const image_probe &probe,
		const image_open_options &options,
		const array_descriptor &descriptor,
		const image_metadata &metadata
	) const
	{
		const auto *format = get_most_suitable_format(probe);
		if (!format)
		{
			throw invalid_operation_error(
				"Could not find a suitable image format to write the "
				"requested file"
			);
		}

		return format->open(probe, options, descriptor, metadata);
	}

private:
	std::vector<std::unique_ptr<image_write_format>> m_formats;

};

image_write_format_manager::image_write_format_manager() noexcept = default;
image_write_format_manager::~image_write_format_manager() = default;

void image_write_format_manager::register_builtin_backends()
{
	get_core_image_write_format_registry().register_all(*this);
}

bool image_write_format_manager::register_format(
	std::unique_ptr<image_write_format> format
)
{
	if (!format)
	{
		return false;
	}

	return create_if_null().register_format(std::move(format));
}

std::unique_ptr<image_writer> image_write_format_manager::open(
	const std::string &path,
	const image_open_options &options,
	const array_descriptor &descriptor,
	const image_metadata &metadata
) const
{
	return get_implementation().open(
		image_probe(path),
		options,
		descriptor,
		metadata
	);
}

const image_write_format*
image_write_format_manager::get_most_suitable_format(
	const image_probe &probe
) const
{
	return get_implementation().get_most_suitable_format(probe);
}

image_write_format_manager::implementation&
image_write_format_manager::create_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}

	return *m_implementation;
}

const image_write_format_manager::implementation&
image_write_format_manager::get_implementation() const noexcept
{
	static const implementation empty_implementation;
	return m_implementation ? *m_implementation : empty_implementation;
}

} // namespace em
} // namespace rexlib
