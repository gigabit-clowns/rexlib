// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_probe.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <utility>

#include <boost/filesystem.hpp>

namespace rexlib
{
namespace em
{

static std::string get_lowercase_extension(const std::string &path)
{
	std::string extension;

	try
	{
		extension = boost::filesystem::path(path).extension().string();
	}
	catch (const boost::filesystem::filesystem_error &)
	{
		return std::string();
	}

	std::transform(
		extension.begin(),
		extension.end(),
		extension.begin(),
		[] (char character) -> char
		{
			return static_cast<char>(
				std::tolower(static_cast<unsigned char>(character))
			);
		}
	);

	return extension;
}

static bool read_header(const std::string &path, std::vector<byte> &header)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
	if (!input.is_open())
	{
		return false;
	}

	header.resize(image_probe::header_size);
	input.read(
		reinterpret_cast<char*>(header.data()),
		static_cast<std::streamsize>(header.size())
	);
	header.resize(static_cast<std::size_t>(input.gcount()));

	return true;
}

image_probe::image_probe(std::string path)
	: m_path(std::move(path))
	, m_extension(get_lowercase_extension(m_path))
	, m_exists(false)
{
	m_exists = read_header(m_path, m_header);
}

image_probe::image_probe(const image_probe &other) = default;
image_probe::image_probe(image_probe &&other) noexcept = default;
image_probe::~image_probe() = default;

image_probe& image_probe::operator=(const image_probe &other) = default;
image_probe& image_probe::operator=(image_probe &&other) noexcept = default;

const std::string& image_probe::get_path() const noexcept
{
	return m_path;
}

const std::string& image_probe::get_extension() const noexcept
{
	return m_extension;
}

span<const byte> image_probe::get_header() const noexcept
{
	return span<const byte>(m_header.data(), m_header.size());
}

bool image_probe::exists() const noexcept
{
	return m_exists;
}

} // namespace em
} // namespace rexlib
