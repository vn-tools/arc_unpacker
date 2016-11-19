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

#include "dec/base_archive_decoder.h"
#include "dec/registry.h"
#include "test_support/catch.h"

using namespace au::dec;

TEST_CASE("Archives reference only valid decoders", "[dec]")
{
    const auto &registry = Registry::instance();
    for (const auto &format_name : registry.get_decoder_names())
    {
        const auto decoder = registry.create_decoder(format_name);
        const auto arc_decoder
            = dynamic_cast<const BaseArchiveDecoder*>(decoder.get());
        if (!arc_decoder)
            continue;
        for (const auto &linked_format_name : arc_decoder->get_linked_formats())
        {
            INFO(format_name
                << " links to invalid format "
                << linked_format_name);
            REQUIRE(registry.has_decoder(linked_format_name));
        }
    }
}
