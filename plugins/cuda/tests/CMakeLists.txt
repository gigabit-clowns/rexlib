cmake_minimum_required(VERSION 3.18)

include(CTest)
include(FetchContent)

# Fetch Catch2
if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW) # To avoid warnings
endif()
FetchContent_Declare(
	Catch2
	URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.0.tar.gz
)
FetchContent_Declare(
	Trompeloeil
	URL https://github.com/rollbear/trompeloeil/archive/refs/tags/v49.tar.gz
)
FetchContent_MakeAvailable(Catch2 Trompeloeil)

include(Catch)

add_subdirectory(unitary)
add_subdirectory(integration)
