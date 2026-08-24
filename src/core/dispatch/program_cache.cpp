// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/dispatch/program_cache.hpp>

#include <rex/core/platform/assert.hpp>
#include <core/logger.hpp>

#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace rexlib
{

class program_cache::implementation
{
public:
	explicit implementation(std::size_t capacity) noexcept
		: m_capacity(capacity)
	{
	}

	std::size_t get_capacity() const noexcept
	{
		return m_capacity;
	}

	std::shared_ptr<void> touch(const program_cache_key &key)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);

		const std::type_index type = typeid(key);
		const auto outer_ite = m_outer_index.find(type);
		if (outer_ite == m_outer_index.end())
		{
			return nullptr;
		}

		const auto &inner = outer_ite->second;
		const auto inner_ite = inner.find(&key);
		if (inner_ite == inner.end())
		{
			return nullptr;
		}

		promote(inner_ite->second);
		return inner_ite->second->value;
	}

	void store(
		std::unique_ptr<const program_cache_key> key,
		std::shared_ptr<void> value
	)
	{
		REXLIB_ASSERT(key);
		REXLIB_ASSERT(value);

		const std::lock_guard<std::mutex> lock(m_mutex);

		const auto &key_ref = *key;
		const std::type_index type = typeid(key_ref);
		auto &inner = m_outer_index[type];
		const auto inner_ite = inner.find(key.get());
		if (inner_ite != inner.end())
		{
			inner_ite->second->value = std::move(value);
			promote(inner_ite->second);
			return;
		}

		if (m_entries.size() >= m_capacity)
		{
			REXLIB_LOG_DEBUG(
				"program_cache::store: at capacity, evicting "
				"least recently used entry"
			);
			evict_oldest();
		}

		m_entries.push_front(
			entry_type{type, std::move(key), std::move(value)}
		);
		inner.emplace(m_entries.front().key.get(), m_entries.begin());
	}

private:
	struct entry_type
	{
		std::type_index type;
		std::unique_ptr<const program_cache_key> key;
		std::shared_ptr<void> value;
	};

	struct key_ptr_hash_type
	{
		std::size_t operator()(
			const program_cache_key *key
		) const noexcept
		{
			REXLIB_ASSERT(key);
			return key->hash();
		}
	};

	struct key_ptr_equal_type
	{
		bool operator()(
			const program_cache_key *lhs,
			const program_cache_key *rhs
		) const noexcept
		{
			REXLIB_ASSERT(lhs);
			REXLIB_ASSERT(rhs);
			return lhs->equals(*rhs);
		}
	};

	using entry_list_type = std::list<entry_type>;
	using index_type = std::unordered_map<
		std::type_index,
		std::unordered_map<
			const program_cache_key*,
			entry_list_type::iterator,
			key_ptr_hash_type,
			key_ptr_equal_type
		>
	>;

	std::mutex m_mutex;
	std::size_t m_capacity;
	entry_list_type m_entries;
	index_type m_outer_index;

	void evict_oldest()
	{
		REXLIB_ASSERT(!m_entries.empty());
		const auto victim = std::prev(m_entries.end());
		const auto outer_ite = m_outer_index.find(victim->type);
		REXLIB_ASSERT(outer_ite != m_outer_index.end());
		outer_ite->second.erase(victim->key.get());
		m_entries.erase(victim);

		if (outer_ite->second.empty())
		{
			m_outer_index.erase(outer_ite);
		}
	}

	void promote(entry_list_type::iterator ite)
	{
		m_entries.splice(m_entries.begin(), m_entries, ite);
	}
};

program_cache::program_cache(
	std::size_t capacity
) noexcept
	: m_implementation(std::make_unique<implementation>(capacity))
{
	REXLIB_ASSERT(capacity > 0);
}

program_cache::~program_cache() = default;

std::size_t program_cache::get_capacity() const noexcept
{
	return m_implementation ? m_implementation->get_capacity() : 0UL;
}

std::shared_ptr<void>
program_cache::touch(const program_cache_key &key)
{
	return m_implementation ? m_implementation->touch(key) : nullptr;
}

void program_cache::store(
	std::unique_ptr<const program_cache_key> key,
	std::shared_ptr<void> value
)
{
	if (!key)
	{
		throw std::invalid_argument(
			"program_cache::store: key must not be null"
		);
	}

	if (!value)
	{
		throw std::invalid_argument(
			"program_cache::store: value must not be null"
		);
	}

	if (m_implementation)
	{
		m_implementation->store(std::move(key), std::move(value));
	}
	else
	{
		REXLIB_LOG_WARN(
			"program_cache::store: dropping entry because the "
			"cache has no backing implementation"
		);
	}
}

} // namespace rexlib
