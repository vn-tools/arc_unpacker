#include "io/file.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::io;

TEST_CASE("Empty file creation", "[core][file]")
{
    File file;
    REQUIRE(file.path == "");
}

TEST_CASE("Setting File's name", "[core][file]")
{
    File file;
    file.path = "abc";
    REQUIRE(file.path == "abc");
}
