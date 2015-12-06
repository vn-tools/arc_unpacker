#include "util/program_path.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Getting program path", "[util]")
{
    const auto program_path = util::get_program_path();
    REQUIRE(program_path.str().find("run_tests") != std::string::npos);
}

TEST_CASE("Getting 'etc/' directory path", "[util]")
{
    const auto etc_path = util::get_etc_dir_path();
    REQUIRE(etc_path.str().find("etc") != std::string::npos);
}
