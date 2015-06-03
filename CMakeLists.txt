cmake_minimum_required(VERSION 2.8)

project(arc_unpacker)

############################
# Files
############################

file(GLOB_RECURSE ProjectSourceFiles src/*.cc)

add_executable(arc_unpacker ${ProjectSourceFiles})

# TODO:
#   headers
#   test

############################
# Dependencies
############################

# Boost
find_package(Boost COMPONENTS filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(arc_unpacker ${Boost_LIBRARIES})


# Other libraries
set(ZLIB_INCLUDE "" CACHE PATH "")
set(ZLIB_LIB_DEBUG "" CACHE FILEPATH "")
set(ZLIB_LIB_RELEASE "" CACHE FILEPATH "")

set(LIBPNG_INCLUDE "" CACHE PATH "")
set(LIBPNG_LIB_DEBUG "" CACHE FILEPATH "")
set(LIBPNG_LIB_RELEASE "" CACHE FILEPATH "")

set(OPENSSL_INCLUDE "" CACHE PATH "")
set(OPENSSL_LIB_DEBUG "" CACHE FILEPATH "")
set(OPENSSL_LIB_RELEASE "" CACHE FILEPATH "")

set(ICONV_INCLUDE "" CACHE PATH "")
set(ICONV_LIB_DEBUG "" CACHE FILEPATH "")
set(ICONV_LIB_RELEASE "" CACHE FILEPATH "")

include_directories(${ZLIB_INCLUDE} ${LIBPNG_INCLUDE} ${OPENSSL_INCLUDE} ${ICONV_INCLUDE})

target_link_libraries(arc_unpacker
  debug ${ZLIB_LIB_DEBUG} optimized ${ZLIB_LIB_RELEASE}
  debug ${LIBPNG_LIB_DEBUG} optimized ${LIBPNG_LIB_RELEASE}
  debug ${OPENSSL_LIB_DEBUG} optimized ${OPENSSL_LIB_RELEASE}
  debug ${ICONV_LIB_DEBUG} optimized ${ICONV_LIB_RELEASE}
)

############################
# Source group for Visual Studio
############################

macro(add_source_group_recursive curdir)
  # GLOB_RECURSIVE not work for directories
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})

      # Recursive call
      add_source_group_recursive(${curdir}/${child})
      file(RELATIVE_PATH relative ${CMAKE_SOURCE_DIR}/src/ ${curdir}/${child})

      # Modify 'relative', replace '/' to '\'
      string(REPLACE "/" "\\" relative ${relative})

      # Add source group for each sub directory
      source_group("src\\${relative}" REGULAR_EXPRESSION "src/${relative}/[^.]+[.]cc$")

    endif()
  endforeach()
endmacro()

add_source_group_recursive("${CMAKE_SOURCE_DIR}/src")

# Group for files in src
source_group("src" REGULAR_EXPRESSION "src/[^.]+[.]cc$")


############################
# Platform dependent
############################

# Windows

if (WIN32)
  # Remove warnings and errors caused by deprecated functions on VC++
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  # Add src to include directories so that compiler can find includes from src.
  include_directories(SYSTEM src)

  # Also need ws2_32.lib
  target_link_libraries(arc_unpacker ws2_32.lib)
endif (WIN32)