cmake_minimum_required(VERSION 2.8.8)
project(arc_unpacker)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/src/cmake")

# -------
# Version
# -------
execute_process(COMMAND git describe --tags OUTPUT_VARIABLE VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git describe --always --dirty --long --tags OUTPUT_VARIABLE VERSION_LONG OUTPUT_STRIP_TRAILING_WHITESPACE)
if("${VERSION_SHORT}" STREQUAL "")
    set(VERSION_SHORT "0.0")
    set(VERSION_LONG "?")
endif()
MESSAGE("Version: ${VERSION_SHORT}")
MESSAGE("Version long: ${VERSION_LONG}")

# ------------
# Dependencies
# ------------
find_package(Boost COMPONENTS filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
link_directories(${ZLIB_LIBRARY_DIRS})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
link_directories(${PNG_LIBRARY_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})
link_directories(${JPEG_LIBRARY_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIBRARY_DIRS})

find_package(WebP)
include_directories(${WEBP_INCLUDE_DIR})
link_directories(${WEBP_LIBRARY_DIRS})

# --------------------
# Global build options
# --------------------
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14") # for MinGW-w64
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffloat-store")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # build in parallel
    add_definitions(-DNOMINMAX)                 # disable retarded macros
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # _wfopen is okay
    add_definitions(-DWINICONV_CONST=)          # win-iconv issue #25
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14") # for stupid travis
endif()

add_definitions(-D_FILE_OFFSET_BITS=64) # 64-bit off_t

if(WEBP_FOUND)
    add_definitions(-DWEBP_FOUND=1)
else()
    add_definitions(-DWEBP_FOUND=0)
endif()

# ------------
# Source files
# ------------
configure_file("${CMAKE_SOURCE_DIR}/src/version.h.in" "${CMAKE_BINARY_DIR}/generated/version.h" @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)

set(CMAKE_SOURCE_DIR "${CMAKE_BINARY_DIR}/../")
file(GLOB_RECURSE au_sources "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE au_headers "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE test_sources "${CMAKE_SOURCE_DIR}/tests/*.cc")
file(GLOB_RECURSE test_headers "${CMAKE_SOURCE_DIR}/tests/*.h")
list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/main.cc")
list(REMOVE_ITEM test_sources "${CMAKE_SOURCE_DIR}/tests/main.cc")

option(micro "Micro" OFF)
function(filter sources)
    foreach(item ${${sources}})
        if(${item} MATCHES "dec/.*/")
            if(NOT ${item} MATCHES "french_bread|/(jpeg|png|bmp|wav)_(audio|image)")
                list(REMOVE_ITEM ${sources} ${item})
            endif(NOT ${item} MATCHES "french_bread|/(jpeg|png|bmp|wav)_(audio|image)")
        endif(${item} MATCHES "dec/.*/")
    endforeach(item ${${sources}})
    set(${sources} ${${sources}} PARENT_SCOPE)
endfunction(filter)
if(micro)
    message("Enabling micro build")
    filter(au_sources)
    filter(au_headers)
    filter(test_sources)
    filter(test_headers)
endif()

if(WIN32)
    list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/logger_ansi.cc")
    list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/logger_dummy.cc")
elseif(CYGWIN OR UNIX)
    list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/logger_win.cc")
    list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/logger_dummy.cc")
else()
    list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/logger_win.cc")
    list(REMOVE_ITEM au_sources "${CMAKE_SOURCE_DIR}/src/logger_ansi.cc")
endif()

macro(group_source_files PARENT_DIR FILES)
    foreach(FILE ${FILES})
        file(RELATIVE_PATH GROUP "${PARENT_DIR}" "${FILE}")
        get_filename_component(GROUP "${GROUP}" PATH)
        string(REPLACE "/" "\\" GROUP "${GROUP}")

        if("${FILE}" MATCHES ".*\\.cc")
            set(GROUP "Source Files\\${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.h")
            set(GROUP "Header Files\\${GROUP}")
        endif()

        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endmacro()

group_source_files("${CMAKE_SOURCE_DIR}/src" "${au_sources};${au_headers}")
group_source_files("${CMAKE_SOURCE_DIR}/tests" "${test_sources};${test_headers}")

# -------------------
# 3rd party libraries
# -------------------

# Catch
set(CATCH_PATH "${CMAKE_SOURCE_DIR}/tests/test_support/catch.h")
if(NOT EXISTS "${CATCH_PATH}")
    message("Downloading Catch...")
    file(DOWNLOAD "https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/catch2/catch.hpp" "${CATCH_PATH}")
endif()

# -------------------
# Linking definitions
# -------------------
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    if(WIN32 OR CYGWIN)
        set(iconv iconv)
    else()
        set(iconv "")
    endif()
    if(WIN32)
        set(unicode "-municode")
    else()
        set(unicode "")
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(iconv iconv)
else()
    set(iconv ${ICONV_LIBRARY})
    set(unicode "")
    include_directories(${ICONV_INCLUDE_DIR})
endif()

add_library(libau OBJECT ${au_sources} ${au_headers})

add_executable(arc_unpacker "${CMAKE_SOURCE_DIR}/src/main.cc" $<TARGET_OBJECTS:libau>)
target_link_libraries(arc_unpacker ${unicode} ${iconv} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${OPENSSL_LIBRARIES})
if(WEBP_FOUND)
    target_link_libraries(arc_unpacker ${WEBP_LIBRARIES})
endif()

add_executable(run_tests ${test_sources} ${test_headers} "${CMAKE_SOURCE_DIR}/tests/main.cc" $<TARGET_OBJECTS:libau>)
target_link_libraries(run_tests ${iconv} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${OPENSSL_LIBRARIES})
if(WEBP_FOUND)
    target_link_libraries(run_tests ${WEBP_LIBRARIES})
endif()

target_include_directories(libau BEFORE PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(libau BEFORE PUBLIC "${CMAKE_BINARY_DIR}/generated")
target_include_directories(arc_unpacker BEFORE PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(arc_unpacker BEFORE PUBLIC "${CMAKE_BINARY_DIR}/generated")
target_include_directories(run_tests BEFORE PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(run_tests BEFORE PUBLIC "${CMAKE_SOURCE_DIR}/tests")
target_include_directories(run_tests BEFORE PUBLIC "${CMAKE_BINARY_DIR}/generated")
