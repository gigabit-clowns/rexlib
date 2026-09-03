// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/caching_image_reader_provider.hpp>

#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace rexlib
{
namespace em
{

class caching_image_reader_provider::implementation
{
public:
	implementation(
		std::shared_ptr<image_reader_provider> backing,
		std::size_t capacity
	)
		: m_backing(std::move(backing))
		, m_capacity(capacity)
	{
	}

	std::size_t get_capacity() const noexcept
	{
		return m_capacity;
	}

	std::size_t get_size() const noexcept
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		return m_entries.size();
	}

	std::shared_ptr<const image_reader> acquire(const std::string &path)
	{
		auto reader = touch(path);
		if (reader)
		{
			return reader;
		}

		reader = m_backing->acquire(path);
		REXLIB_ASSERT(reader);
		store(path, reader);
		return reader;
	}

private:
	struct entry_type
	{
		std::string path;
		std::shared_ptr<const image_reader> reader;
	};

	using entry_list_type = std::list<entry_type>;

	mutable std::mutex m_mutex;
	std::shared_ptr<image_reader_provider> m_backing;
	std::size_t m_capacity;
	entry_list_type m_entries;
	std::unordered_map<std::string, entry_list_type::iterator> m_index;

	std::shared_ptr<const image_reader> touch(const std::string &path)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto ite = m_index.find(path);
		if (ite == m_index.end())
		{
			return nullptr;
		}

		m_entries.splice(m_entries.begin(), m_entries, ite->second);
		return ite->second->reader;
	}

	void store(
		const std::string &path,
		const std::shared_ptr<const image_reader> &reader
	)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto ite = m_index.find(path);
		if (ite != m_index.end())
		{
			ite->second->reader = reader;
			m_entries.splice(m_entries.begin(), m_entries, ite->second);
			return;
		}

		if (m_entries.size() >= m_capacity)
		{
			evict_oldest();
		}

		m_entries.push_front(entry_type{path, reader});
		m_index.emplace(path, m_entries.begin());
	}

	void evict_oldest()
	{
		REXLIB_ASSERT(!m_entries.empty());
		const auto victim = std::prev(m_entries.end());
		m_index.erase(victim->path);
		m_entries.erase(victim);
	}
};

caching_image_reader_provider::caching_image_reader_provider(
	std::shared_ptr<image_reader_provider> backing,
	std::size_t capacity
)
{
	if (!backing)
	{
		throw std::invalid_argument(
			"caching_image_reader_provider: The backing provider must not "
			"be null."
		);
	}

	if (capacity == 0)
	{
		throw std::invalid_argument(
			"caching_image_reader_provider: The capacity must be greater "
			"than zero."
		);
	}

	m_implementation.reset(
		new implementation(std::move(backing), capacity)
	);
}

caching_image_reader_provider::~caching_image_reader_provider() = default;

std::size_t caching_image_reader_provider::get_capacity() const noexcept
{
	return m_implementation->get_capacity();
}

std::size_t caching_image_reader_provider::get_size() const noexcept
{
	return m_implementation->get_size();
}

std::shared_ptr<const image_reader>
caching_image_reader_provider::acquire(const std::string &path)
{
	return m_implementation->acquire(path);
}

} // namespace em
} // namespace rexlib
