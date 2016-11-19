// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include <regex>
#include "algo/str.h"
#include "dec/registry.h"
#include "io/file_byte_stream.h"
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
    io::FileByteStream gamelist_file("GAMELIST.js", io::FileMode::Read);
    return gamelist_file.read_to_eof().str();
}

TEST_CASE("Documentation", "[core]")
{
    SECTION("--dec switches contain hyphens rather than underscores")
    {
        const auto &registry = dec::Registry::instance();
        for (const auto &name : registry.get_decoder_names())
        {
            INFO("Decoder name contains underscore: " << name);
            REQUIRE(name.find_first_of("_") == name.npos);
        }
    }

    SECTION("GAMELIST refers to valid --dec switches")
    {
        const auto content = read_gamelist_file();
        const auto &registry = dec::Registry::instance();

        const std::regex decoder_name_regex(
            "--dec=([^' ]*)",
            std::regex_constants::ECMAScript | std::regex_constants::icase);

        for (const auto decoder_name
            : regex_range(decoder_name_regex, content, 1))
        {
            INFO("Decoder name not present in the registry: " << decoder_name);
            REQUIRE(registry.has_decoder(decoder_name));
        }
    }
}
