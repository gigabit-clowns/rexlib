// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "core/hardware/memory_allocator_table.hpp"

#include "mock/mock_device.hpp"
#include "mock/mock_memory_allocator.hpp"
#include "mock/mock_memory_resource.hpp"

using namespace rexlib;

TEST_CASE(
    "memory_allocator_table default constructor leaves all slots null",
    "[memory_allocator_table]"
)
{
    memory_allocator_table set;

    CHECK(set.get_allocator(memory_resource_affinity::host) == nullptr);
    CHECK(set.get_allocator(memory_resource_affinity::device) == nullptr);
}

TEST_CASE(
    "memory_allocator_table device constructor creates one allocator "
    "per distinct memory resource",
    "[memory_allocator_table]"
)
{
    auto host_allocator = std::make_shared<mock_memory_allocator>();
    auto device_allocator = std::make_shared<mock_memory_allocator>();

    mock_memory_resource host_resource;
    mock_memory_resource device_resource;

    REQUIRE_CALL(host_resource, create_allocator())
        .RETURN(host_allocator);
    REQUIRE_CALL(device_resource, create_allocator())
        .RETURN(device_allocator);

    mock_device dev;
    REQUIRE_CALL(dev, get_memory_resource(memory_resource_affinity::host))
        .LR_RETURN(host_resource);
    REQUIRE_CALL(dev, get_memory_resource(memory_resource_affinity::device))
        .LR_RETURN(device_resource);

    memory_allocator_table set(dev);

    CHECK(set.get_allocator(memory_resource_affinity::host) == host_allocator);
    CHECK(
        set.get_allocator(memory_resource_affinity::device) == device_allocator
    );
}

TEST_CASE(
    "memory_allocator_table device constructor shares one allocator "
    "when all affinities map to the same memory resource",
    "[memory_allocator_table]"
)
{
    auto shared_allocator = std::make_shared<mock_memory_allocator>();

    mock_memory_resource shared_resource;
    REQUIRE_CALL(shared_resource, create_allocator())
        .RETURN(shared_allocator);

    mock_device dev;
    REQUIRE_CALL(dev, get_memory_resource(memory_resource_affinity::host))
        .LR_RETURN(shared_resource);
    REQUIRE_CALL(dev, get_memory_resource(memory_resource_affinity::device))
        .LR_RETURN(shared_resource);

    memory_allocator_table set(dev);

    CHECK(
        set.get_allocator(memory_resource_affinity::host) == shared_allocator
    );
    CHECK(
        set.get_allocator(memory_resource_affinity::device) == shared_allocator
    );
    CHECK(
        set.get_allocator(memory_resource_affinity::host)
        == set.get_allocator(memory_resource_affinity::device)
    );
}

TEST_CASE(
    "memory_allocator_table set_allocator installs the new allocator",
    "[memory_allocator_table]"
)
{
    memory_allocator_table set;

    auto allocator = std::make_shared<mock_memory_allocator>();
    set.set_allocator(memory_resource_affinity::host, allocator);

    CHECK(set.get_allocator(memory_resource_affinity::host) == allocator);
}

TEST_CASE(
    "memory_allocator_table set_allocator returns the previous allocator",
    "[memory_allocator_table]"
)
{
    memory_allocator_table set;

    auto first = std::make_shared<mock_memory_allocator>();
    auto second = std::make_shared<mock_memory_allocator>();

    set.set_allocator(memory_resource_affinity::device, first);
    auto previous =
        set.set_allocator(memory_resource_affinity::device, second);

    CHECK(previous == first);
    CHECK(set.get_allocator(memory_resource_affinity::device) == second);
}

TEST_CASE(
    "memory_allocator_table set_allocator returns nullptr when slot "
    "was empty",
    "[memory_allocator_table]"
)
{
    memory_allocator_table set;

    auto allocator = std::make_shared<mock_memory_allocator>();
    auto previous =
        set.set_allocator(memory_resource_affinity::host, allocator);

    CHECK(previous == nullptr);
}

TEST_CASE(
    "memory_allocator_table set_allocator with nullptr clears the slot",
    "[memory_allocator_table]"
)
{
    memory_allocator_table set;

    auto allocator = std::make_shared<mock_memory_allocator>();
    set.set_allocator(memory_resource_affinity::host, allocator);
    auto previous =
        set.set_allocator(memory_resource_affinity::host, nullptr);

    CHECK(previous == allocator);
    CHECK(set.get_allocator(memory_resource_affinity::host) == nullptr);
}

TEST_CASE(
    "memory_allocator_table set_allocator on one affinity does not "
    "affect the other",
    "[memory_allocator_table]"
)
{
    memory_allocator_table set;

    auto host_allocator = std::make_shared<mock_memory_allocator>();
    set.set_allocator(memory_resource_affinity::host, host_allocator);

    CHECK(set.get_allocator(memory_resource_affinity::device) == nullptr);
}

TEST_CASE(
    "memory_allocator_table move constructor transfers all slots",
    "[memory_allocator_table]"
)
{
    memory_allocator_table source;

    auto host_allocator = std::make_shared<mock_memory_allocator>();
    auto device_allocator = std::make_shared<mock_memory_allocator>();
    source.set_allocator(memory_resource_affinity::host, host_allocator);
    source.set_allocator(memory_resource_affinity::device, device_allocator);

    memory_allocator_table destination(std::move(source));

    CHECK(
        destination.get_allocator(memory_resource_affinity::host)
        == host_allocator
    );
    CHECK(
        destination.get_allocator(memory_resource_affinity::device)
        == device_allocator
    );
}

TEST_CASE(
    "memory_allocator_table move assignment transfers all slots",
    "[memory_allocator_table]"
)
{
    memory_allocator_table source;

    auto host_allocator = std::make_shared<mock_memory_allocator>();
    auto device_allocator = std::make_shared<mock_memory_allocator>();
    source.set_allocator(memory_resource_affinity::host, host_allocator);
    source.set_allocator(memory_resource_affinity::device, device_allocator);

    memory_allocator_table destination;
    destination = std::move(source);

    CHECK(
        destination.get_allocator(memory_resource_affinity::host)
        == host_allocator
    );
    CHECK(
        destination.get_allocator(memory_resource_affinity::device)
        == device_allocator
    );
}
