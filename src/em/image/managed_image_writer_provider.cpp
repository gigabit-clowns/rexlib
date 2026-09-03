// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/managed_image_writer_provider.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/platform/assert.hpp>
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
		span<const std::size_t> extents,
		std::size_t core_rank,
		numerical_type data_type,
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

		entry_type entry;
		entry.extents.assign(extents.begin(), extents.end());
		entry.metadata = metadata;
		entry.core_rank = core_rank;
		entry.data_type = data_type;
		m_files.emplace(std::move(path), std::move(entry));
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

			writer = std::move(ite->second.writer);
			m_files.erase(ite);
		}

		// Outside the lock: a flush reaches the storage, and no other file
		// needs to wait for it.
		if (writer)
		{
			writer->flush();
		}
	}

	std::size_t get_size() const noexcept
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

		auto &entry = ite->second;
		if (!entry.writer)
		{
			entry.writer = m_formats->open(
				ite->first,
				make_span(entry.extents.data(), entry.extents.size()),
				entry.core_rank,
				entry.data_type,
				entry.metadata
			);
			REXLIB_ASSERT(entry.writer);
		}

		return entry.writer;
	}

	void flush()
	{
		std::vector<std::shared_ptr<image_writer>> writers;
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			writers.reserve(m_files.size());
			for (const auto &file : m_files)
			{
				if (file.second.writer)
				{
					writers.push_back(file.second.writer);
				}
			}
		}

		for (const auto &writer : writers)
		{
			writer->flush();
		}
	}

private:
	// What a file was declared as, kept until it is created. The path is
	// the key it is stored under.
	struct entry_type
	{
		std::vector<std::size_t> extents;
		image_metadata metadata;
		std::size_t core_rank = 0;
		numerical_type data_type = numerical_type::unknown;
		std::shared_ptr<image_writer> writer;
	};

	mutable std::mutex m_mutex;
	std::shared_ptr<const image_write_format_manager> m_formats;
	std::unordered_map<std::string, entry_type> m_files;
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

	m_implementation.reset(new implementation(std::move(formats)));
}

managed_image_writer_provider::~managed_image_writer_provider() = default;

void managed_image_writer_provider::declare(
	std::string path,
	span<const std::size_t> extents,
	std::size_t core_rank,
	numerical_type data_type,
	const image_metadata &metadata
)
{
	// Checked here rather than left to the format, so that a core rank that
	// names no image or volume is refused where it was written down and not
	// at whatever later point the file is first acquired.
	if (core_rank == 0)
	{
		throw std::invalid_argument(
			"managed_image_writer_provider::declare: The core rank must not "
			"be zero."
		);
	}

	if (core_rank > extents.size())
	{
		throw std::invalid_argument(
			"managed_image_writer_provider::declare: The core rank must not "
			"exceed the rank of the extents."
		);
	}

	m_implementation->declare(
		std::move(path),
		extents,
		core_rank,
		data_type,
		metadata
	);
}

void managed_image_writer_provider::close(const std::string &path)
{
	m_implementation->close(path);
}

std::size_t managed_image_writer_provider::get_size() const noexcept
{
	return m_implementation->get_size();
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
