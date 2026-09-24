// SPDX-License-Identifier: GPL-3.0-only

#include "rethrow_with_path.hpp"

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <exception>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

std::string name_file(const std::string &path, const std::exception &error)
{
	return path + ": " + error.what();
}

} // anonymous namespace

void rethrow_with_path(const std::string &path)
{
	try
	{
		throw;
	}
	catch (const image_file_error &error)
	{
		throw image_file_error(name_file(path, error));
	}
	catch (const image_format_error &error)
	{
		throw image_format_error(name_file(path, error));
	}
	catch (const unsupported_operation_error &error)
	{
		throw unsupported_operation_error(name_file(path, error));
	}
	catch (const unsupported_capability_error &error)
	{
		throw unsupported_capability_error(name_file(path, error));
	}
	catch (const std::out_of_range &error)
	{
		throw std::out_of_range(name_file(path, error));
	}
	catch (const std::invalid_argument &error)
	{
		throw std::invalid_argument(name_file(path, error));
	}
}

} // namespace em
} // namespace rexlib
