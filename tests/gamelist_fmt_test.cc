#include <regex>
#include "fmt/registry.h"
#include "io/file_io.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("GAMELIST refers to valid --fmt switches", "[core][docs]")
{
    io::FileIO gamelist_file("GAMELIST.htm", io::FileMode::Read);
    const auto content = gamelist_file.read_to_eof().str();
    const auto &registry = fmt::Registry::instance();

    std::regex fmt_regex(
        "--fmt=([^< ]*)",
        std::regex_constants::ECMAScript | std::regex_constants::icase);

    auto it = std::sregex_iterator(content.begin(), content.end(), fmt_regex);
    const auto end = std::sregex_iterator();
    while (it != end)
    {
        const auto fmt = (*it)[1];
        INFO("Format not present in the registry: " << fmt);
        REQUIRE(registry.has_decoder(fmt));
        it++;
    }
}
