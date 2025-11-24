cmake_minimum_required(VERSION 3.16)

include(CTest)
include(FetchContent)

# Fetch Catch2
cmake_policy(SET CMP0135 NEW) # To avoid warnings
FetchContent_Declare(
	Catch2
	URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.11.0.tar.gz
)
FetchContent_Declare(
	Trompeloeil
	URL https://github.com/rollbear/trompeloeil/archive/refs/tags/v49.tar.gz
)
FetchContent_MakeAvailable(Catch2 Trompeloeil)

include(Catch)

# add_subdirectory(unitary) # TODO uncomment
# add_subdirectory(integration) # TODO uncomment
