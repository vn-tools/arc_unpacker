cmake_minimum_required(VERSION 2.8)

project(arc_unpacker)

############################
# Files
############################

file(GLOB_RECURSE LibSourceFiles ../src/*.cc)
file(GLOB_RECURSE LibHeaderFiles ../src/*.h)
file(GLOB_RECURSE TestSourceFiles ../tests/*.cc)
file(GLOB_RECURSE TestHeaderFiles ../tests/*.h)

# Remove main.cc from lib
list(REMOVE_ITEM LibSourceFiles "../src/main.cc")

# Library (shared by arc_unpacker and arc_unpacker_test)
add_library(lib_arc_unpacker ${LibSourceFiles} ${LibHeaderFiles})

# Executable
add_executable(arc_unpacker "../src/main.cc")
target_link_libraries(arc_unpacker lib_arc_unpacker)

# Test
add_executable(arc_unpacker_test ${TestSourceFiles} ${TestHeaderFiles})
target_link_libraries(arc_unpacker_test lib_arc_unpacker)

############################
# Dependencies
############################

# Boost
find_package(Boost COMPONENTS filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(lib_arc_unpacker ${Boost_LIBRARIES})

# Other libraries
set(ZLIB_INCLUDE "" CACHE PATH "")
set(ZLIB_LIB_DEBUG "" CACHE FILEPATH "")
set(ZLIB_LIB_RELEASE "" CACHE FILEPATH "")

set(LIBPNG_INCLUDE "" CACHE PATH "")
set(LIBPNG_LIB_DEBUG "" CACHE FILEPATH "")
set(LIBPNG_LIB_RELEASE "" CACHE FILEPATH "")

# TODO make this optional
set(OPENSSL_INCLUDE "" CACHE PATH "")
set(OPENSSL_LIB_DEBUG "" CACHE FILEPATH "")
set(OPENSSL_LIB_RELEASE "" CACHE FILEPATH "")
add_definitions(-DHAVE_OPENSSL_RSA_H)

set(ICONV_INCLUDE "" CACHE PATH "")
set(ICONV_LIB_DEBUG "" CACHE FILEPATH "")
set(ICONV_LIB_RELEASE "" CACHE FILEPATH "")

include_directories(${ZLIB_INCLUDE} ${LIBPNG_INCLUDE} ${OPENSSL_INCLUDE} ${ICONV_INCLUDE})

target_link_libraries(lib_arc_unpacker
  debug ${ZLIB_LIB_DEBUG} optimized ${ZLIB_LIB_RELEASE}
  debug ${LIBPNG_LIB_DEBUG} optimized ${LIBPNG_LIB_RELEASE}
  debug ${OPENSSL_LIB_DEBUG} optimized ${OPENSSL_LIB_RELEASE}
  debug ${ICONV_LIB_DEBUG} optimized ${ICONV_LIB_RELEASE}
)

############################
# Platform dependent
############################

# Visual Studio Source Goup
if(MSVC)
  macro(add_source_group_recursive curdir)
    # GLOB_RECURSIVE not work for directories
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    foreach(child ${children})
      if(IS_DIRECTORY ${curdir}/${child})

        # Recursive call
        add_source_group_recursive(${curdir}/${child})
        file(RELATIVE_PATH relative ${CMAKE_SOURCE_DIR}/../src/ ${curdir}/${child})


        # Modify 'relative', replace '/' to '\'
        string(REPLACE "/" "\\" group_name ${relative})

        # Add source group for each sub directory
        source_group("src\\${group_name}" REGULAR_EXPRESSION "../src/${relative}/[^./]+[.]cc$")
        source_group("header\\${group_name}" REGULAR_EXPRESSION "../src/${relative}/[^./]+[.]h$")
        source_group("src\\${group_name}" REGULAR_EXPRESSION "../tests/${relative}/[^./]+[.]cc$")
        source_group("header\\${group_name}" REGULAR_EXPRESSION "../tests/${relative}/[^./]+[.]h$")

      endif()
    endforeach()
  endmacro()

  add_source_group_recursive("${CMAKE_SOURCE_DIR}/../src")
  # Group for files in src
  source_group("src" REGULAR_EXPRESSION "../src/[^.]+[.]cc$")
  source_group("header" REGULAR_EXPRESSION "../src/[^.]+[.]h$")
  source_group("src" REGULAR_EXPRESSION "../tests/[^.]+[.]cc$")
  source_group("header" REGULAR_EXPRESSION "../tests/[^.]+[.]h$")
endif()

# Warnings
if(MSVC)
  # Remove warnings and errors caused by deprecated functions on VC++
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# Build
if(MSVC)
  # CMake can add libraries automatically
  add_definitions(-DBOOST_ALL_NO_LIB)

  # Add src to include directories so that compiler can find includes from src.
  include_directories(SYSTEM ../src)
  include_directories(SYSTEM ../tests)

  # Also need ws2_32.lib
  target_link_libraries(lib_arc_unpacker ws2_32.lib)
endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -iquote${CMAKE_SOURCE_DIR}/../src")
endif()