# Copyright 2019, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

# The root CMakeLists.txt of Boost's superproject, boostorg/boost, for the
# libraries checked out in libs/, with the version that Boost.Config states.

cmake_minimum_required(VERSION 3.8...3.16)

file(
  STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/libs/config/include/boost/version.hpp
  boost_version REGEX "^#define BOOST_VERSION [0-9]+$"
)
string(REGEX MATCH "[0-9]+$" boost_version "${boost_version}")
math(EXPR boost_version_major "${boost_version} / 100000")
math(EXPR boost_version_minor "${boost_version} / 100 % 1000")
math(EXPR boost_version_patch "${boost_version} % 100")

project(
  Boost
  VERSION ${boost_version_major}.${boost_version_minor}.${boost_version_patch}
  LANGUAGES CXX
)

set(BOOST_SUPERPROJECT_VERSION ${PROJECT_VERSION})
set(BOOST_SUPERPROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tools/cmake/include)

include(BoostRoot)
