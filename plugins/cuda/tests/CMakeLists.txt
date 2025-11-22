cmake_minimum_required(VERSION 3.16)

include(CTest)
include(FetchContent)

# Fetch Catch2
FetchContent_Declare(
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

# Fetch Trompeloeil
FetchContent_Declare(
	Trompeloeil
	GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
	GIT_TAG        v49
)
FetchContent_MakeAvailable(Trompeloeil)

include(Catch)

# add_subdirectory(unitary) # TODO uncomment
# add_subdirectory(integration) # TODO uncomment
