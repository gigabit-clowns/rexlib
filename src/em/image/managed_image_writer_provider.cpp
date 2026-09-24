// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/managed_image_writer_provider.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

class managed_image_writer_provider::implementation
{
public:
	explicit implementation(
		std::shared_ptr<const image_write_format_manager> formats
	)
		: m_formats(std::move(formats))
	{
	}

	void declare(
		std::string path,
		image_descriptor descriptor,
		const image_metadata &metadata
	)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto ite = m_files.find(path);
		if (ite != m_files.end())
		{
			throw invalid_operation_error(
				"managed_image_writer_provider::declare: That path is "
				"already declared."
			);
		}

		m_files.emplace(
			std::move(path),
			declared_file(std::move(descriptor), metadata)
		);
	}

	void close(const std::string &path)
	{
		std::shared_ptr<image_writer> writer;
		{
			const std::lock_guard<std::mutex> lock(m_mutex);

			const auto ite = m_files.find(path);
			if (ite == m_files.end())
			{
				throw std::out_of_range(
					"managed_image_writer_provider::close: That path is not "
					"declared."
				);
			}

			writer = ite->second.get_writer();
			m_files.erase(ite);
		}

		// Outside the lock: a flush reaches the storage, and no other file
		// needs to wait for it.
		if (writer)
		{
			writer->flush();
		}
	}

	std::size_t get_file_count() const noexcept
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		return m_files.size();
	}

	std::shared_ptr<image_writer> acquire(const std::string &path)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto ite = m_files.find(path);
		if (ite == m_files.end())
		{
			throw std::out_of_range(
				"managed_image_writer_provider::acquire: That path is not "
				"declared."
			);
		}

		auto &file = ite->second;
		if (!file.get_writer())
		{
			file.set_writer(
				m_formats->open(
					ite->first,
					file.get_descriptor(),
					file.get_metadata()
				)
			);
			REXLIB_ASSERT(file.get_writer());
		}

		return file.get_writer();
	}

	void flush()
	{
		std::vector<std::shared_ptr<image_writer>> writers;
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			writers.reserve(m_files.size());
			for (const auto &file : m_files)
			{
				if (file.second.get_writer())
				{
					writers.push_back(file.second.get_writer());
				}
			}
		}

		for (const auto &writer : writers)
		{
			writer->flush();
		}
	}

private:
	// What a file was declared as, and its writer once it is created. The
	// path is the key it is stored under.
	class declared_file
	{
	public:
		declared_file(image_descriptor descriptor, image_metadata metadata)
			: m_descriptor(std::move(descriptor))
			, m_metadata(std::move(metadata))
		{
		}

		const image_descriptor& get_descriptor() const noexcept
		{
			return m_descriptor;
		}

		const image_metadata& get_metadata() const noexcept
		{
			return m_metadata;
		}

		const std::shared_ptr<image_writer>& get_writer() const noexcept
		{
			return m_writer;
		}

		void set_writer(std::shared_ptr<image_writer> writer) noexcept
		{
			m_writer = std::move(writer);
		}

	private:
		image_descriptor m_descriptor;
		image_metadata m_metadata;
		std::shared_ptr<image_writer> m_writer;
	};

	mutable std::mutex m_mutex;
	std::shared_ptr<const image_write_format_manager> m_formats;
	std::unordered_map<std::string, declared_file> m_files;
};

managed_image_writer_provider::managed_image_writer_provider(
	std::shared_ptr<const image_write_format_manager> formats
)
{
	if (!formats)
	{
		throw std::invalid_argument(
			"managed_image_writer_provider: The format manager must not be "
			"null."
		);
	}

	m_implementation = std::make_unique<implementation>(std::move(formats));
}

managed_image_writer_provider::~managed_image_writer_provider() = default;

void managed_image_writer_provider::declare(
	std::string path,
	image_descriptor descriptor,
	const image_metadata &metadata
)
{
	m_implementation->declare(
		std::move(path),
		std::move(descriptor),
		metadata
	);
}

void managed_image_writer_provider::close(const std::string &path)
{
	m_implementation->close(path);
}

std::size_t managed_image_writer_provider::get_file_count() const noexcept
{
	return m_implementation->get_file_count();
}

std::shared_ptr<image_writer>
managed_image_writer_provider::acquire(const std::string &path)
{
	return m_implementation->acquire(path);
}

void managed_image_writer_provider::flush()
{
	m_implementation->flush();
}

} // namespace em
} // namespace rexlib
