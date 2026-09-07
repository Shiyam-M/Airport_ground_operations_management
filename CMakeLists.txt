cmake_minimum_required(VERSION 3.16)
project(AirportGroundOperations CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

# ---------------------------------------------------------------------------
# SQLite3
#
# This project links against the system SQLite3 shared library. It ships a
# minimal, hand-written subset of the SQLite C API declarations in
# third_party/sqlite3/sqlite3_min.h (see that file for a full explanation)
# so that it builds even on systems that only have the runtime library
# (libsqlite3-0 / libsqlite3.so[.0]) and not the "-dev" package with the
# official sqlite3.h header.
#
# If the official header/dev package IS available, this still works
# unmodified -- CMake will find and link libsqlite3 normally, and the
# hand-written declarations in sqlite3_min.h are ABI-compatible with it.
# ---------------------------------------------------------------------------
find_library(SQLITE3_LIBRARY NAMES sqlite3 sqlite3.so.0
             PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

if(NOT SQLITE3_LIBRARY)
    # Fall back to searching for the unversioned/major-versioned .so
    # directly, which is present even without libsqlite3-dev installed.
    file(GLOB SQLITE3_SO_CANDIDATES
         /usr/lib/x86_64-linux-gnu/libsqlite3.so*
         /usr/lib/libsqlite3.so*
         /usr/lib64/libsqlite3.so*)
    if(SQLITE3_SO_CANDIDATES)
        list(GET SQLITE3_SO_CANDIDATES 0 SQLITE3_LIBRARY)
    endif()
endif()

if(NOT SQLITE3_LIBRARY)
    message(FATAL_ERROR "Could not locate libsqlite3 on this system. "
                         "Install it with e.g. 'sudo apt-get install libsqlite3-0' "
                         "or 'sudo apt-get install libsqlite3-dev'.")
endif()

message(STATUS "Using SQLite3 library: ${SQLITE3_LIBRARY}")

# ---------------------------------------------------------------------------
# Core library: all model / service / repository / controller sources,
# compiled once and shared by both the main executable and the test suite.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE AGOMS_CORE_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_library(agoms_core STATIC ${AGOMS_CORE_SOURCES})
target_include_directories(agoms_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/sqlite3)
target_link_libraries(agoms_core PUBLIC ${SQLITE3_LIBRARY})

# ---------------------------------------------------------------------------
# Main console application
# ---------------------------------------------------------------------------
add_executable(agoms_app main.cpp)
target_link_libraries(agoms_app PRIVATE agoms_core)

# ---------------------------------------------------------------------------
# Tests (lightweight custom framework -- see tests/TestFramework.h)
#
# GoogleTest is intentionally NOT used via FetchContent here because this
# project is built to also work in fully offline / sandboxed environments.
# tests/TestFramework.h implements the small subset of assertion macros the
# test suite needs. If you have GoogleTest available and prefer it, the
# test_*.cpp files use a trivial macro-based API that would be simple to
# port to gtest's TEST()/ASSERT_* macros.
# ---------------------------------------------------------------------------
option(AGOMS_BUILD_TESTS "Build the AGOMS test suite" ON)
if(AGOMS_BUILD_TESTS)
    file(GLOB AGOMS_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")
    add_executable(agoms_tests ${AGOMS_TEST_SOURCES})
    target_include_directories(agoms_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
    target_link_libraries(agoms_tests PRIVATE agoms_core)
endif()
