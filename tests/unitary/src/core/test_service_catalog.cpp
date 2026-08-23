// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/service_catalog.hpp>

#include "mock/mock_service_manager.hpp"

#include <catch2/catch_test_macros.hpp>

#include <utility>

using namespace rex;

namespace
{

class counting_service_manager : public service_manager
{
public:
	void register_builtin_backends() override
	{
		++m_register_count;
	}

	int get_register_count() const noexcept
	{
		return m_register_count;
	}

private:
	int m_register_count = 0;
};

class other_counting_service_manager : public service_manager
{
public:
	void register_builtin_backends() override
	{
		++m_register_count;
	}

	int get_register_count() const noexcept
	{
		return m_register_count;
	}

private:
	int m_register_count = 0;
};

} // namespace

TEST_CASE(
	"service_catalog returns the same instance on repeated access",
	"[service_catalog]"
)
{
	service_catalog catalog(false);
	const auto first = catalog.get_service_manager<mock_service_manager>();
	const auto second = catalog.get_service_manager<mock_service_manager>();

	REQUIRE(first);
	CHECK(first == second);
}

TEST_CASE(
	"service_catalog hands each service_manager type its own instance",
	"[service_catalog]"
)
{
	service_catalog catalog(false);
	const auto first =
		catalog.get_service_manager<counting_service_manager>();
	const auto second =
		catalog.get_service_manager<other_counting_service_manager>();

	REQUIRE(first);
	REQUIRE(second);
	// Different types must not be aliased under a shared catalog key.
	CHECK(
		static_cast<service_manager *>(first.get())
		!= static_cast<service_manager *>(second.get())
	);
}

TEST_CASE(
	"service_catalog calls register_builtin_backends on first access "
	"when enabled",
	"[service_catalog]"
)
{
	service_catalog catalog(true);
	const auto manager =
		catalog.get_service_manager<counting_service_manager>();

	REQUIRE(manager);
	CHECK(manager->get_register_count() == 1);
}

TEST_CASE(
	"service_catalog skips register_builtin_backends when disabled",
	"[service_catalog]"
)
{
	service_catalog catalog(false);
	// mock_service_manager has no expectations set, so any call to
	// register_builtin_backends would trigger a trompeloeil violation.
	const auto manager =
		catalog.get_service_manager<mock_service_manager>();

	REQUIRE(manager);
}

TEST_CASE(
	"service_catalog only calls register_builtin_backends on first access",
	"[service_catalog]"
)
{
	service_catalog catalog(true);
	const auto first =
		catalog.get_service_manager<counting_service_manager>();
	const auto second =
		catalog.get_service_manager<counting_service_manager>();

	REQUIRE(first);
	REQUIRE(first == second);
	CHECK(first->get_register_count() == 1);
}

TEST_CASE(
	"service_catalog preserves managers through move construction",
	"[service_catalog]"
)
{
	service_catalog source(false);
	const auto manager =
		source.get_service_manager<mock_service_manager>();

	service_catalog destination(std::move(source));
	const auto reloaded =
		destination.get_service_manager<mock_service_manager>();

	CHECK(reloaded == manager);
}

TEST_CASE(
	"service_catalog preserves managers through move assignment",
	"[service_catalog]"
)
{
	service_catalog source(false);
	const auto manager =
		source.get_service_manager<mock_service_manager>();

	service_catalog destination(false);
	destination = std::move(source);
	const auto reloaded =
		destination.get_service_manager<mock_service_manager>();

	CHECK(reloaded == manager);
}
