// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_location.hpp>

#include <sstream>
#include <utility>

#include <boost/functional/hash.hpp>

namespace rexlib
{
namespace em
{

namespace
{

bool parse_index_in_stack(
	const char *begin,
	const char *end,
	std::size_t &result
) noexcept
{
	if (begin == end)
	{
		return false;
	}

	std::size_t value = 0;
	for (auto ite = begin; ite != end; ++ite)
	{
		if (*ite < '0' || *ite > '9')
		{
			return false;
		}

		const auto digit = static_cast<std::size_t>(*ite - '0');
		if (value > (image_location::no_stack_index - digit) / 10)
		{
			return false;
		}

		value = (value * 10) + digit;
	}

	if (value == 0)
	{
		return false;
	}

	result = value - 1;
	return true;
}

} // anonymous namespace

image_location::image_location() noexcept
	: m_index_in_stack(no_stack_index)
{
}

image_location::image_location(std::string path, std::size_t index_in_stack)
	: m_path(std::move(path))
	, m_index_in_stack(index_in_stack)
{
}

image_location::image_location(const image_location &other) = default;
image_location::image_location(image_location &&other) noexcept = default;
image_location::~image_location() = default;

image_location&
image_location::operator=(const image_location &other) = default;
image_location&
image_location::operator=(image_location &&other) noexcept = default;

std::size_t image_location::hash() const noexcept
{
	auto seed = boost::hash_value(m_path);
	boost::hash_combine(seed, boost::hash_value(m_index_in_stack));
	return seed;
}

const std::string& image_location::get_path() const noexcept
{
	return m_path;
}

std::size_t image_location::get_index_in_stack() const noexcept
{
	return m_index_in_stack;
}

bool image_location::has_index_in_stack() const noexcept
{
	return m_index_in_stack != no_stack_index;
}

bool parse_image_location(const std::string &text, image_location &result)
{
	if (text.empty())
	{
		return false;
	}

	const char separator = '@';
	const auto position = text.find(separator);
	if (position == std::string::npos)
	{
		result = image_location(text);
		return true;
	}

	if (position == 0 || (position + 1) == text.size())
	{
		return false;
	}

	std::size_t index;
	const auto begin = text.data();
	if (!parse_index_in_stack(begin, begin + position, index))
	{
		return false;
	}

	result = image_location(text.substr(position + 1), index);
	return true;
}

std::string to_string(const image_location &location)
{
	std::ostringstream output;
	output << location;
	return output.str();
}

} // namespace em
} // namespace rexlib
