#include <regex>
#include "algo/str.h"
#include "dec/registry.h"
#include "io/file_stream.h"
#include "test_support/catch.h"

using namespace au;

namespace
{
    struct regex_range final
    {
        regex_range(
            const std::regex &r, const std::string &content, const int group) :
                content_copy(content),
                it(content_copy.begin(), content_copy.end(), r, group)
        {
        }

        std::sregex_token_iterator begin()
        {
            return it;
        }

        std::sregex_token_iterator end()
        {
            return _end;
        }

        std::string content_copy;
        std::sregex_token_iterator it;
        std::sregex_token_iterator _end;
    };
}

static std::string read_gamelist_file()
{
    io::FileStream gamelist_file("GAMELIST.htm", io::FileMode::Read);
    return gamelist_file.read_to_eof().str();
}

TEST_CASE("Documentation", "[core]")
{
    SECTION("--fmt switches contain hyphens rather than underscores")
    {
        const auto &registry = dec::Registry::instance();
        for (const auto &name : registry.get_decoder_names())
        {
            INFO("Decoder name contains underscore: " << name);
            REQUIRE(name.find_first_of("_") == name.npos);
        }
    }

    SECTION("GAMELIST refers to valid --fmt switches")
    {
        const auto content = read_gamelist_file();
        const auto &registry = dec::Registry::instance();

        const std::regex decoder_name_regex(
            "--fmt=([^< ]*)",
            std::regex_constants::ECMAScript | std::regex_constants::icase);

        for (const auto decoder_name
            : regex_range(decoder_name_regex, content, 1))
        {
            INFO("Decoder name not present in the registry: " << decoder_name);
            REQUIRE(registry.has_decoder(decoder_name));
        }
    }

    SECTION("GAMELIST is sorted alphabetically")
    {
        const auto content = read_gamelist_file();
        const std::regex row_regex(
            "<tr>(([\r\n]|.)*?)</tr>", std::regex_constants::ECMAScript);
        const std::regex cell_regex(
            "<td>(.*)</td>", std::regex_constants::ECMAScript);

        size_t comparisons = 0;
        std::string last_sort_key;
        for (const auto row : regex_range(row_regex, content, 1))
        {
            std::vector<std::string> cells;
            for (const auto cell : regex_range(cell_regex, row, 1))
                cells.push_back(cell);
            if (cells.size() != 7)
                continue;
            const auto company = cells[3];
            const auto release_date = cells[4];
            const auto game_title = cells[5];
            const auto sort_key = algo::lower(
                "[" + company + "][" + release_date + "]" + game_title);
            REQUIRE(sort_key > last_sort_key);
            last_sort_key = sort_key;
            comparisons++;
        }

        // sanity check to see if regexes actually work and match stuff
        REQUIRE(comparisons > 10);
    }
}
