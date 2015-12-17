#include "io/program_path.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Getting program path", "[io]")
{
    const auto program_path = io::get_program_path();
    REQUIRE(program_path.str().find("run_tests") != std::string::npos);
}

TEST_CASE("Getting assets directory path", "[io]")
{
    const auto assets_dir_path = io::get_assets_dir_path();
    REQUIRE(assets_dir_path.str().find("etc") != std::string::npos);
}
