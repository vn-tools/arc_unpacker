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

#include "dec/almond_collective/pac3_archive_decoder.h"
#include <array>
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::almond_collective;

static const bstr magic = "PAC3"_b;

static const bstr sboxes[4] =
{
    "\x49\x4D\xC2\x63\xCF\x5B\x3F\x0A\xC0\xF6\x76\x1A\x82\xAF\xB9\xC6"
    "\x6D\x9F\x57\xD6\xF8\x42\x2A\xFD\xB0\xB2\xBE\x3D\x9E\x39\xD5\xE8"
    "\xF1\x3B\x88\x4A\x1D\xAC\xDE\x34\x65\x16\x74\x47\x86\xDA\x51\xA8"
    "\xF2\xE0\x5F\x94\xE1\x81\x4B\xF3\x8B\x91\xEF\x3A\x03\xF4\xA7\x7F"
    "\xB5\xE4\x02\x99\x68\x9A\xBC\x8C\xB1\x56\xBA\x8D\x93\xD8\x23\x70"
    "\x8A\xBD\x90\xE2\xB6\xF5\x43\x07\x1C\xFA\x48\x14\x00\xD9\x1B\x9B"
    "\x46\xFB\xAB\x30\x28\xA4\x53\xA3\xB7\xC8\x2E\xEC\x78\xC4\x55\x61"
    "\xA9\x98\x9D\xBF\x8E\xE6\xDF\x85\x6F\x44\xCA\x80\x4F\x25\x5C\xA2"
    "\xED\x6E\xA6\xC7\xE3\x0F\xAD\x83\xEA\x12\x87\xBB\xFF\x05\xF9\x06"
    "\xDC\xB3\xE7\x95\x37\x01\x5E\x32\x7A\x4C\x60\x59\xD1\x2B\x77\x2D"
    "\xA0\xF0\x7D\x21\x96\x7B\xC1\x15\x73\x66\xDD\x17\xC5\x2C\x5A\x09"
    "\xB8\x33\x3C\x11\xE5\xAA\x0D\xD3\x72\x50\x29\xC9\xEE\x40\x69\x36"
    "\x08\x6B\x79\xAE\x67\x0C\x45\xEB\x20\x64\x52\xDB\x71\xB4\xCE\x19"
    "\x58\x6A\xCB\x9C\x0E\x89\x84\x7C\xA5\xC3\x62\x5D\xF7\x22\x8F\xD0"
    "\xE9\x97\x13\x35\x24\xD4\xCC\x27\x38\x04\x1F\x92\x7E\x18\xA1\x26"
    "\xD7\x41\x54\xCD\x10\x2F\xD2\x31\x3E\x4E\x1E\xFE\x75\x0B\xFC\x6C"_b,

    "\xC4\xDA\x33\xBD\x32\x71\x76\x7E\xB7\x16\x0F\x30\x90\x1A\xE4\x1C"
    "\x9D\x24\xC1\xD4\x2B\xCC\xEC\x5B\xC7\xE8\xA8\x2C\x8D\xD5\x68\x0D"
    "\xAC\x21\xFB\x52\x42\x06\x07\xBE\xDD\xFF\xD8\x1D\x01\x1B\x50\xE1"
    "\x12\x58\xC2\x41\x8A\x85\x82\x1F\x10\x57\xAE\x45\x9A\xA2\xB0\x4C"
    "\x70\xED\x3B\x9F\xF2\xDB\xDE\x9C\xE2\xF7\xB1\xBC\xE6\xB9\xCA\xD9"
    "\x6F\xB5\x9B\x53\xDC\x20\x5A\x0E\xA5\xBB\x95\x7C\xAA\x3F\x2E\x34"
    "\xAF\x79\xD0\x8C\x51\xA0\x00\x7A\x2D\xBA\x6B\x5F\x55\x0A\x48\xBF"
    "\xCD\x5E\xE3\xB6\xF1\x78\xA9\xB3\x7D\x4D\xE5\x99\x29\x37\xEA\x14"
    "\x03\x75\x66\xA7\xCE\x6E\xB8\x93\xD3\xF3\xA3\x81\x46\x28\xFC\x96"
    "\x38\xEE\x2F\x27\x8F\x36\x5D\xD7\xD1\xA4\xFA\x3C\x54\xF8\xF9\x11"
    "\x25\x40\xEF\xB2\x0C\xF4\x18\x0B\xC9\x3E\xC8\x17\x98\x09\x74\x73"
    "\x56\x49\xF0\x35\x59\x4B\x7F\x05\x2A\x94\xEB\x9E\xE0\x13\x6D\x65"
    "\x4A\xE7\x08\xC5\x02\x15\xC3\x69\x87\x04\x86\x88\xA6\xAD\x91\xF6"
    "\x39\x3D\xCF\x31\xDF\x6A\x47\xF5\x4F\xB4\x8B\x6C\xE9\xAB\x8E\xC0"
    "\x1E\x89\x3A\xA1\x77\x7B\x84\x5C\x63\xCB\xC6\x4E\x19\xFD\x23\x22"
    "\xD2\xFE\x83\x62\xD6\x72\x97\x44\x67\x60\x92\x80\x26\x64\x43\x61"_b,

    "\x84\xC7\x92\x1A\xA7\x07\xA6\xEE\xB6\xB3\x7B\xC2\xD3\xDC\x75\x46"
    "\x93\xFD\x39\x3A\xED\xE3\x32\x77\x5C\x3E\xD0\x7E\xB0\x89\x6B\x6F"
    "\xCD\xDF\xE8\x45\x4E\xFC\x4F\xB8\x8E\x53\xFE\xD1\x42\x6C\xEB\xB5"
    "\xF7\xCC\x51\x0D\x8B\x8D\x90\x34\x08\xE6\x72\x3C\x14\x6D\x76\xEF"
    "\xAD\xC6\x68\x94\xE1\x2A\xCA\x5F\x4D\x6A\xBD\x19\x54\xCB\x15\xC8"
    "\x18\x1C\x48\x97\x9C\x41\x37\x04\xD5\xAC\xB9\x11\xF3\xD2\xF1\xEC"
    "\xBB\x0C\xE9\x8C\x67\x78\x65\x4B\xDA\x59\xF5\x86\x60\xA9\x16\xD9"
    "\x21\x83\xE2\x9D\xFB\x70\x2B\xB7\x17\xC4\xFA\x12\x29\x06\xB4\x30"
    "\x31\x79\xD7\x1B\x20\x87\xE4\xD8\x57\x2E\x09\x10\xF4\xFF\x40\x73"
    "\xA3\x69\x99\xAB\x96\x1D\xAF\xA5\x23\x02\xB2\xC9\x24\x56\x52\xC3"
    "\x2F\x05\x58\x55\x5B\x22\xF2\x61\xF8\x38\x8A\xC0\x3D\x03\x9A\x66"
    "\x44\xE5\x35\xDE\x80\xCE\x5E\x50\x9E\x43\x82\x62\x33\xB1\x25\xBF"
    "\xD4\x2C\x9B\xD6\x8F\x4C\xAA\xDB\x9F\x00\x74\x5A\xF6\x26\x0E\x1E"
    "\xA2\x7F\xA4\x85\x7A\x7D\x13\xF0\x3F\x91\xC1\x36\x28\x71\x5D\xBE"
    "\x4A\xF9\x27\xBA\xCF\x6E\x81\x64\xA8\x0A\x01\xBC\xC5\x63\xE0\x49"
    "\xA0\xA1\xAE\x3B\x98\x47\xDD\x1F\x7C\xEA\x0F\x2D\x95\x88\x0B\xE7"_b,

    "\x63\x5D\x3F\xCF\x78\x10\x02\xCC\xA2\xB3\x0E\xCE\x46\x55\xE6\xA0"
    "\x4D\xE0\x01\x39\xCA\x3A\xC5\xAC\xB2\x19\xAF\xA7\x36\xE2\x5E\x8C"
    "\x26\x15\x22\x62\x73\x00\xE9\x75\x0B\x6E\x58\x91\xED\xA1\xD9\x44"
    "\xD4\x86\xA8\x20\xEA\x16\x2E\x79\x81\xBD\x74\x1F\x3D\xB0\x33\xFB"
    "\x97\x95\x51\x4F\x7F\x8B\x05\x38\xC1\x90\x98\x72\x50\x9E\x1C\x17"
    "\xC2\x45\xF0\xC4\xE7\xF4\xB8\x2C\xF5\xEF\xAE\x7C\x4C\xF9\x23\x5B"
    "\x4E\x0C\x7B\xA9\x7E\xF8\x9C\x25\x0D\x5F\x1E\x35\x6C\xF1\x67\xA5"
    "\x99\x57\x6D\xDF\xA4\x40\xA3\xD7\x7A\x07\x06\xDD\xBE\x9F\x8A\xD0"
    "\xA6\x94\x92\xC0\x88\x77\xFE\x53\xF2\x03\xC6\xFF\xE5\x80\x12\xC7"
    "\xF7\xC8\x49\x5A\x29\x08\x28\x1B\xB4\x82\x21\xB7\x47\xF6\x34\x6F"
    "\x31\xCB\xBB\x66\x85\x48\x3B\x65\x3C\xB9\x43\xAB\x8D\xC3\xB1\x3E"
    "\x93\xDC\xBA\xE1\x96\x56\xD5\x87\x1A\xE8\x41\xAD\x54\xE4\x52\x71"
    "\xB6\x83\xD6\xC9\xBC\x37\xCD\x0A\x09\x69\xB5\xF3\x8F\x2B\x2F\xDE"
    "\x76\x14\xEE\xD2\x1D\x89\x42\xFD\x61\xE3\xD1\xDB\x6A\x70\x5C\x04"
    "\x30\x7D\xAA\xDA\x64\x9D\xEB\x2D\x4B\x8E\x0F\x32\x27\xEC\x2A\xD3"
    "\xD8\x84\x4A\x68\xFA\x11\x60\x18\x13\xBF\xFC\x59\x9A\x24\x6B\x9B"_b,
};

namespace
{
    using BlockKey = std::array<u8, 256>;

    struct FileKey final
    {
        BlockKey key0;
        BlockKey key1;
        BlockKey key2;
        u8 marker;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        FileKey key;
    };
}

static BlockKey expand_key(const bstr &input)
{
    BlockKey key = {0};

    size_t input_sum = 0;
    for (const u8 &c : input)
        input_sum += c;

    u32 buffer[0x50] = {0};
    for (const auto i : algo::range(input.size()))
        reinterpret_cast<u8*>(buffer)[i] = input[i];

    const u8 *sbox = sboxes[input_sum % 4].get<const u8>();
    for (const auto round : algo::range(4, 0x50, 4))
    {
        u32 *ptr = buffer + round;
        u32 prev = ptr[-1];
        for (const auto i : algo::range(4))
        {
            if (!((static_cast<u8>(round) + i) & 3))
            {
                u32 tmp2 = algo::rotl<u32>(prev, 8);
                u32 tmp = 0;
                for (const auto j : algo::range(4))
                    tmp |= sbox[(tmp2 >> (j << 3)) & 0xFF] << (j << 3);
                prev = ((1 << ((round + i) & 7)) | 0x30303000) ^ tmp;
            }
            prev ^= ptr[i - 4];
            ptr[i] = prev;
        }
    }

    const u8 *buffer_u8 = reinterpret_cast<u8*>(buffer);
    auto *key_ptr = key.data();
    for (const auto i : algo::range(16))
        for (const auto j : algo::range(i + 1))
            *key_ptr++ = buffer_u8[256 + i - j * 17];

    for (const auto i : algo::range(15))
        for (const auto j : algo::range((14 - i) + 1))
            *key_ptr++ = buffer_u8[255 - i * 16 - j * 17];

    return key;
}

static void decrypt_table_chunk(
    size_t stream_position,
    u8 *target,
    const size_t target_size,
    const BlockKey &key)
{
    auto left = target_size;
    auto key_left = 256u - stream_position % 256;
    auto *target_ptr = target;
    const auto *key_ptr = &key[stream_position % 256];
    while (left)
    {
        if (key_left >= left)
            key_left = left;

        // below is a saner implementation of:
        // for (const auto i : algo::range(key_left >> 2))
        // {
        //     *target_ptr ^= *key_ptr;
        //     *reinterpret_cast<u32*>(target_ptr) ^=
        //         *reinterpret_cast<const u32*>(key_ptr);
        //     target_ptr += 4;
        //     key_ptr += 4;
        // }
        for (const auto i : algo::range(key_left & ~3))
        {
            if (i & 3)
                *target_ptr ^= *key_ptr;
            target_ptr++;
            key_ptr++;
        }

        for (const auto i : algo::range(key_left & 3))
            *target_ptr++ ^= *key_ptr++;

        left -= key_left;
        key_ptr = &key[0];
        key_left = 256;
    }
}

static bstr read_and_decrypt(
    io::BaseByteStream &input_stream, const size_t size, const BlockKey &key)
{
    const auto original_pos = input_stream.pos();
    auto ret = input_stream.read(size);
    decrypt_table_chunk(original_pos, ret.get<u8>(), ret.size(), key);
    return ret;
}

template<typename T> static T read_and_decrypt_le(
    io::BaseByteStream &input_stream, const BlockKey &key)
{
    const auto original_pos = input_stream.pos();
    auto ret = input_stream.read_le<T>();
    decrypt_table_chunk(
        original_pos, reinterpret_cast<u8*>(&ret), sizeof(T), key);
    return ret;
}

static FileKey create_file_key(
    const unsigned int file_id,
    const size_t file_size,
    const std::string &file_name,
    const u32 key)
{
    // const auto tmp0 = ((key >> 25) & 0x7F) + 1980;
    // const auto tmp1 = static_cast<u16>(
    //     static_cast<double>(365 * tmp0)
    //     + static_cast<double>(tmp0) * 0.25
    //     - static_cast<double>(tmp0) * 0.01
    //     + static_cast<double>(
    //         static_cast<signed int>(
    //             306 * (((key >> 21) & 0xF) + 1))) * 0.1
    //     + static_cast<double>((key >> 16) & 0x1F)
    //     - 428.0);

    char transient1[0x20] = {0};
    snprintf(
        transient1,
        0x20,
        "%02d%02d%02d%02d%02d%02d%04X",
        key & 0x3F,
        (key >> 6) & 0x3F,
        (key >> 12) & 0xF,
        (key >> 16) & 0x1F,
        (key >> 21) & 0xF,
        (key >> 25) & 0x7F,
        static_cast<u16>(file_size));

    bstr transient2(16);
    std::string reversed_file_name(file_name);
    std::reverse(reversed_file_name.begin(), reversed_file_name.end());
    for (const auto i : algo::range(16))
    {
        transient2[i]
            = transient1[i]
            ^ reversed_file_name[i % reversed_file_name.size()];
    }

    const auto expanded_key = expand_key(transient2);

    FileKey file_key;
    file_key.marker = 0;
    for (const auto i : algo::range(256))
        file_key.key0[i] = i;
    for (const auto &tuple : expanded_key)
        std::swap(file_key.key0[tuple & 0xF], file_key.key0[tuple >> 4]);
    file_key.key1 = expanded_key;
    file_key.key2 = expanded_key;
    return file_key;
}

static void mix_key(
    BlockKey &target,
    const BlockKey &source,
    const size_t marker,
    const size_t target_size)
{
    if (!marker)
    {
        for (const auto i : algo::range(target_size))
            target[i] = source[i];
        return;
    }

    if (target_size >> 1 == marker)
    {
        u8 *target_ptr = &target[target_size - 1];
        const u8 *source_ptr = &source[0];
        for (const auto i : algo::range(target_size))
            *target_ptr-- = *source_ptr++;
        return;
    }

    u8 *target_left_ptr = &target[marker];
    u8 *target_right_ptr = &target[target_size - marker - 1];
    const u8 *source_left_ptr = &source[0];
    const u8 *source_right_ptr = &source[target_size - 1];
    if (marker < target_size)
    {
        for (const auto i : algo::range(((target_size - marker - 1) >> 1) + 1))
        {
            *target_left_ptr = *source_left_ptr;
            *target_right_ptr = *source_right_ptr;
            source_left_ptr += 2;
            target_left_ptr += 2;
            source_right_ptr -= 2;
            target_right_ptr -= 2;
        }
    }

    target_left_ptr -= target_size;
    target_right_ptr += target_size;
    if (marker <= 1)
        return;

    for (const auto i : algo::range(((marker - 2) >> 1) + 1))
    {
        *target_left_ptr = *source_left_ptr;
        *target_right_ptr = *source_right_ptr;
        source_left_ptr += 2;
        target_left_ptr += 2;
        source_right_ptr -= 2;
        target_right_ptr -= 2;
    }
}

static void decrypt_file_data(
    FileKey &file_key,
    const size_t stream_position,
    bstr &input)
{
    const u8 expected_marker = stream_position >> 16;
    if (file_key.marker != expected_marker)
    {
        mix_key(file_key.key2, file_key.key1, expected_marker, 0x100u);
        file_key.marker = expected_marker;
    }

    auto left = input.size();
    u8 *input_ptr = input.get<u8>();
    const u8 *key0_ptr = &file_key.key0[(stream_position >> 8) % 256];
    const u8 *key2_ptr = &file_key.key2[stream_position % 256];
    auto key2_left = 256u - stream_position % 256;
    auto position = stream_position;

    while (left)
    {
        if (key2_left >= left)
            key2_left = left;

        const auto key0 = *key0_ptr++;
        for (const auto i : algo::range(key2_left))
            *input_ptr++ ^= key0 ^ *key2_ptr++;

        position += key2_left;
        left -= key2_left;
        key2_ptr = &file_key.key2[0];
        key2_left = 256;

        if (!(position & 0xFFFF))
        {
            ++file_key.marker;
            key0_ptr = &file_key.key0[0];
            mix_key(file_key.key2, file_key.key1, file_key.marker, 0x100u);
        }
    }
}

Pac3ArchiveDecoder::Pac3ArchiveDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--pac3-key"})
                ->set_value_name("KEY")
                ->set_description("Decryption key");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("pac3-key"))
                game_key = arg_parser.get_switch("pac3-key");
        });
}

bool Pac3ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pac3ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(4);

    if (game_key.empty())
        throw err::UsageError("Please supply game key with --pac3-key switch.");

    const auto key = expand_key(game_key);
    const auto unk1 = read_and_decrypt(input_file.stream, 16, key);
    const auto file_count = read_and_decrypt_le<u32>(input_file.stream, key);
    const auto unk2 = read_and_decrypt_le<u32>(input_file.stream, key);
    const auto table_size = read_and_decrypt_le<u32>(input_file.stream, key);

    io::MemoryByteStream table_stream(
        read_and_decrypt(input_file.stream, table_size, key));
    auto current_offset = input_file.stream.pos();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto file_id = table_stream.read_le<u32>();
        entry->offset = current_offset;
        entry->size = table_stream.read_le<u32>();
        table_stream.skip(4);
        const auto key = table_stream.read_le<u32>();
        const auto name_size = table_stream.read<u8>();
        const auto name = table_stream.read_to_zero(name_size).str(true);
        entry->path = algo::sjis_to_utf8(name).str();
        entry->key = create_file_key(file_id, entry->size, name, key);
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pac3ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    FileKey file_key_copy = entry->key;
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    decrypt_file_data(file_key_copy, entry->offset, data);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<Pac3ArchiveDecoder>(
    "almond-collective/pac3");
