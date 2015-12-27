#include "fmt/base_archive_decoder.h"
#include "fmt/registry.h"
#include "test_support/catch.h"

using namespace au::fmt;

TEST_CASE("Archives reference only valid decoders", "[fmt_core]")
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
