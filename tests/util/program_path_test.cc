#include "test_support/catch.hh"
#include "util/program_path.h"

using namespace au::util;

TEST_CASE("Getting program path", "[util]")
{
    REQUIRE(get_program_path().str().find("run_tests") != std::string::npos);
}

TEST_CASE("Getting 'etc/' directory path", "[util]")
{
    REQUIRE(get_etc_dir_path().str().find("etc") != std::string::npos);
}
