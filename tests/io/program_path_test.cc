#include "io/program_path.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Getting program path", "[io]")
{
    const auto program_path = io::get_program_path();
    REQUIRE(program_path.str().find("run_tests") != std::string::npos);
}

TEST_CASE("Getting 'etc/' directory path", "[io]")
{
    const auto etc_path = io::get_etc_dir_path();
    REQUIRE(etc_path.str().find("etc") != std::string::npos);
}
