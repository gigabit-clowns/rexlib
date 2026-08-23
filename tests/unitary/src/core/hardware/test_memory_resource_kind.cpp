// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/hardware/memory_resource_kind.hpp>

using namespace rex;

TEST_CASE(
	"score_host_affinity should return the expected score for each kind",
	"[memory_resource_kind]"
)
{
    REQUIRE( score_host_affinity(memory_resource_kind::host)          == 2 );
    REQUIRE( score_host_affinity(memory_resource_kind::host_staging)  == 2 );
    REQUIRE( score_host_affinity(memory_resource_kind::device_mapped) == 2 );
    REQUIRE( score_host_affinity(memory_resource_kind::unified)       == 2 );
    REQUIRE( score_host_affinity(memory_resource_kind::managed)       == 1 );
    REQUIRE( score_host_affinity(memory_resource_kind::device_local)  == 0 );
}

TEST_CASE(
	"score_device_affinity should return the expected score for each kind",
	"[memory_resource_kind]"
)
{
    REQUIRE( score_device_affinity(memory_resource_kind::device_local)  == 4 );
    REQUIRE( score_device_affinity(memory_resource_kind::unified)       == 3 );
    REQUIRE( score_device_affinity(memory_resource_kind::managed)       == 2 );
    REQUIRE( score_device_affinity(memory_resource_kind::device_mapped) == 1 );
    REQUIRE( score_device_affinity(memory_resource_kind::host_staging)  == 0 );
    REQUIRE( score_device_affinity(memory_resource_kind::host)          == 0 );
}

TEST_CASE(
	"is_host_accessible should return true iff score_host_affinity > 0",
	"[memory_resource_kind]"
)
{
    REQUIRE(  is_host_accessible(memory_resource_kind::host) );
    REQUIRE(  is_host_accessible(memory_resource_kind::host_staging) );
    REQUIRE(  is_host_accessible(memory_resource_kind::device_mapped) );
    REQUIRE(  is_host_accessible(memory_resource_kind::unified) );
    REQUIRE(  is_host_accessible(memory_resource_kind::managed) );
    REQUIRE( !is_host_accessible(memory_resource_kind::device_local) );
}

TEST_CASE(
	"is_device_accessible should return true iff score_device_affinity > 0",
	"[memory_resource_kind]"
)
{
    REQUIRE(  is_device_accessible(memory_resource_kind::device_local) );
    REQUIRE(  is_device_accessible(memory_resource_kind::unified) );
    REQUIRE(  is_device_accessible(memory_resource_kind::managed) );
    REQUIRE(  is_device_accessible(memory_resource_kind::device_mapped) );
    REQUIRE( !is_device_accessible(memory_resource_kind::host_staging) );
    REQUIRE( !is_device_accessible(memory_resource_kind::host) );
}
