#include "dec/kaguya/common/params_encryption.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::dec::kaguya::common;

static void write_string(io::BaseByteStream &stream, const bstr &str)
{
    stream.write<u8>(str.size());
    stream.write(str);
}

TEST_CASE("Atelier Kaguya params decryption", "[dec]")
{
    const auto key = "\x01\x02\x03"_b;
    const auto game_title = "game title"_b;
    io::MemoryStream output_stream;

    // for unknown data, serialize nonempty junk to test skipping

    SECTION("Version 2")
    {
        output_stream.write("[SCR-PARAMS]v02"_b);
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>('?');
        write_string(output_stream, "???"_b);
        write_string(output_stream, game_title);
        write_string(output_stream, "producer"_b);
        write_string(output_stream, "copyright"_b);
        write_string(output_stream, "???"_b);
        output_stream.write<u8>('?');
        write_string(output_stream, "???"_b);
        write_string(output_stream, "???"_b);

        const std::vector<std::pair<bstr, bstr>> arc_names
            = {{"bla"_b, "bla"_b}, {"herp"_b, "derp"_b}};
        output_stream.write<u8>(arc_names.size());
        for (const auto &kv : arc_names)
        {
            write_string(output_stream, kv.first);
            write_string(output_stream, kv.second);
        }

        const std::vector<std::vector<bstr>> unk = {
            {"12"_b, "3"_b, "45"_b, "67"_b, "89"_b},
            {"123"_b, "45"_b, "67"_b, "89"_b}};
        const std::vector<bstr> unk3 = {"12"_b, "3"_b, "45"_b};
        const std::vector<bstr> unk4 = {"123"_b, "45"_b};
        output_stream.write<u8>('?');
        output_stream.write<u8>(unk.size());
        for (const auto &t : unk)
        {
            output_stream.write<u8>('?');
            write_string(output_stream, "???"_b);
            output_stream.write<u8>(t.size());
            for (const auto &tt : t)
                write_string(output_stream, tt);
            write_string(output_stream, "???"_b);
        }
        output_stream.write<u8>(unk3.size());
        for (const auto &s : unk3)
            write_string(output_stream, s);
        output_stream.write<u8>(unk4.size());
        for (const auto &s : unk4)
            write_string(output_stream, s);
        output_stream.write_le<u32>(key.size());
        output_stream.write(key);
    }

    const auto params = parse_params_file(output_stream);
    REQUIRE(params.key == key);
    REQUIRE(params.game_title == game_title);
}
