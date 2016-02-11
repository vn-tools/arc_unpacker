#include "dec/kaguya/common/params_encryption.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

common::Params common::parse_params_file(io::BaseByteStream &input_stream)
{
    const auto scr_magic = input_stream.seek(0).read(15);
    if (scr_magic != "[SCR-PARAMS]v02"_b)
        throw err::RecognitionError();

    input_stream.skip(8);

    input_stream.skip(input_stream.read<u8>());
    const auto game_title
        = algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
    const auto producer
        = algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
    const auto copyright
        = algo::sjis_to_utf8(input_stream.read(input_stream.read<u8>()));
    input_stream.skip(input_stream.read<u8>());
    input_stream.skip(1);

    for (const auto i : algo::range(2))
        input_stream.skip(input_stream.read<u8>());
    for (const auto i : algo::range(input_stream.read<u8>()))
    {
        const auto arc_name = input_stream.read(input_stream.read<u8>());
        const auto arc_type = input_stream.read(input_stream.read<u8>());
    }
    input_stream.skip(1);

    size_t key_size;

    if (game_title == "幼なじみと甘～くエッチに過ごす方法"_b
        || game_title == "艶女医"_b)
    {
        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(1);
            input_stream.skip(input_stream.read<u8>());
            for (const auto j : algo::range(input_stream.read<u8>()))
                input_stream.skip(input_stream.read<u8>());
            input_stream.skip(input_stream.read<u8>());
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
            input_stream.skip(input_stream.read<u8>());

        for (const auto i : algo::range(input_stream.read<u8>()))
            input_stream.skip(input_stream.read<u8>());

        key_size = input_stream.read_le<u32>();
        if (game_title == "幼なじみと甘～くエッチに過ごす方法"_b)
            key_size = 240000;
    }

    else if (game_title == "新妻イカせてミルク！"_b
        || game_title == "毎日がＭ！"_b)
    {
        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(1);
            input_stream.skip(input_stream.read<u8>());
            for (const auto j : algo::range(input_stream.read<u8>()))
                input_stream.skip(input_stream.read<u8>());
            for (const auto j : algo::range(input_stream.read<u8>()))
                input_stream.skip(input_stream.read<u8>());
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(input_stream.read<u8>());
            input_stream.skip(input_stream.read<u8>());
        }

        for (const auto i : algo::range(input_stream.read<u8>()))
        {
            input_stream.skip(input_stream.read<u8>());
            for (const auto j : algo::range(input_stream.read<u8>()))
                input_stream.skip(input_stream.read<u8>());
            for (const auto j : algo::range(input_stream.read<u8>()))
                input_stream.skip(input_stream.read<u8>());
        }

        key_size = input_stream.read_le<u32>();
    }
    else
    {
        throw err::CorruptDataError("Unknown game: " + game_title.str());
    }

    common::Params params;
    params.game_title = game_title;
    params.key = input_stream.read(key_size);
    return params;
}

static void decrypt(
    io::BaseByteStream &input_stream,
    const bstr &key,
    const size_t pos,
    const size_t size)
{
    const auto old_pos = input_stream.tell();
    input_stream.seek(pos);
    const auto data = algo::unxor(input_stream.read(size), key);
    input_stream.seek(pos);
    input_stream.write(data);
    input_stream.seek(old_pos);
}

static void decrypt(
    io::BaseByteStream &input_stream, const bstr &key, const size_t pos)
{
    decrypt(input_stream, key, pos, input_stream.size() - pos);
}

void common::decrypt(
    io::BaseByteStream &input_stream, const common::Params &params)
{
    if (input_stream.seek(0).read(2) == "BM"_b)
        ::decrypt(input_stream, params.key, 54);

    if (input_stream.seek(0).read(4) == "AP-0"_b)
        ::decrypt(input_stream, params.key, 12);

    if (input_stream.seek(0).read(4) == "AP-1"_b)
        ::decrypt(input_stream, params.key, 12);

    if (input_stream.seek(0).read(4) == "AP-2"_b)
        ::decrypt(input_stream, params.key, 24);

    if (input_stream.seek(0).read(4) == "AP-3"_b)
        ::decrypt(input_stream, params.key, 24);

    if (input_stream.seek(0).read(4) == "AN00"_b)
    {
        if (params.game_title == "幼なじみと甘～くエッチに過ごす方法"_b)
            return;

        input_stream.seek(20);
        const auto frame_count = input_stream.read_le<u16>();
        input_stream.skip(2 + frame_count * 4);
        const auto file_count = input_stream.read_le<u16>();
        for (const auto i : algo::range(file_count))
        {
            input_stream.skip(8);
            const auto width = input_stream.read_le<u32>();
            const auto height = input_stream.read_le<u32>();
            const auto size = 4 * width * height;
            ::decrypt(input_stream, params.key, input_stream.tell(), size);
            input_stream.skip(size);
        }
    }
}
