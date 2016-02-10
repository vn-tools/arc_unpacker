#include "dec/kaguya/common/params_encryption.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

bstr common::get_key_from_params_file(io::BaseByteStream &input_stream)
{
    const auto scr_magic = input_stream.seek(0).read(15);
    if (scr_magic != "[SCR-PARAMS]v02"_b)
        throw err::RecognitionError();

    input_stream.skip(8);

    const auto game_title
        = algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
    input_stream.skip(input_stream.read<u8>());
    const auto producer
        = algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
    const auto copyright
        = algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
    input_stream.skip(input_stream.read<u8>());
    input_stream.skip(1);

    for (const auto i : algo::range(2))
        input_stream.skip(input_stream.read<u8>());
    const auto arc_count = input_stream.read<u8>();
    for (const auto i : algo::range(arc_count))
    {
        const auto arc_name = input_stream.read(input_stream.read<u8>());
        const auto arc_type = input_stream.read(input_stream.read<u8>());
    }
    input_stream.skip(1);

    const auto unk_count = input_stream.read<u8>();
    for (const auto i : algo::range(unk_count))
    {
        input_stream.skip(1);
        input_stream.skip(input_stream.read<u8>());
        const auto unk2_count = input_stream.read<u8>();
        for (const auto j : algo::range(unk2_count))
            input_stream.skip(input_stream.read<u8>());
        input_stream.skip(input_stream.read<u8>());
    }

    const auto unk3_count = input_stream.read<u8>();
    for (const auto i : algo::range(unk3_count))
        input_stream.skip(input_stream.read<u8>());

    const auto unk4_count = input_stream.read<u8>();
    for (const auto i : algo::range(unk4_count))
        input_stream.skip(input_stream.read<u8>());

    const auto key_size = input_stream.read_le<u32>();
    return input_stream.read(key_size);
}

int common::get_encryption_offset(const bstr &data)
{
    if (data.substr(0, 2) == "BM"_b) return 54;
    if (data.substr(0, 4) == "AP-0"_b) return 12;
    if (data.substr(0, 4) == "AP-1"_b) return 12;
    if (data.substr(0, 4) == "AP-2"_b) return 24;
    if (data.substr(0, 4) == "AP-3"_b) return 24;
    return -1;
}
