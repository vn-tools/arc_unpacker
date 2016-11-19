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

#include "algo/range.h"
#include "io/lsb_bit_stream.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"
#include "test_support/catch.h"

using namespace au;

namespace
{
    enum class TestType : u8
    {
        Msb,
        Lsb,
    };
}

static bstr from_bits(std::initializer_list<u8> s)
{
    bstr x;
    for (const u8 &c : s)
        x += c;
    return x;
}

template<class T> static void test_reading_missing_bits()
{
    SECTION("Reading missing bits throws exceptions")
    {
        T reader(""_b);
        REQUIRE_THROWS(reader.read(1));
    }
}

template<class T> static void test_reading_single_bits(const TestType type)
{
    SECTION("Reading single bits")
    {
        T reader("\x8F"_b); // 10001111
        std::vector<u8> actual_value;
        for (const auto i : algo::range(8))
            actual_value.push_back(reader.read(1));
        const auto expected_value = type == TestType::Msb
            ? std::vector<u8>{1, 0, 0, 0, 1, 1, 1, 1}
            : std::vector<u8>{1, 1, 1, 1, 0, 0, 0, 1};
        REQUIRE((actual_value == expected_value));
    }
}

template<class T> static void test_reading_multiple_bits(const TestType type)
{
    SECTION("Reading multiple bits")
    {
        T reader(from_bits({0b10001111}));
        const std::vector<u32> actual_value = {reader.read(7), reader.read(1)};
        const auto expected_value = type == TestType::Msb
            ? std::vector<u32>{0b1000111, 1}
            : std::vector<u32>{0b0001111, 1};
        REQUIRE((actual_value == expected_value));
    }
}

template<class T> static void test_reading_multiple_bytes(const TestType type)
{
    SECTION("Reading multiple bytes")
    {
        SECTION("Smaller test")
        {
            T reader("\x8F\x8F"_b); // 10001111 10001111
            if (type == TestType::Msb)
            {
                REQUIRE((reader.read(7) == (0x8F >> 1)));
                REQUIRE(reader.read(1));
                REQUIRE(reader.read(1));
                REQUIRE(!reader.read(1));
                REQUIRE((reader.read(4) == 3));
                REQUIRE((reader.read(2) == 3));
            }
            else
            {
                REQUIRE(reader.read(1));
                REQUIRE((reader.read(7) == (0x8F >> 1)));
                REQUIRE(reader.read(1));
                REQUIRE(reader.read(1));
                REQUIRE((reader.read(2) == 3));
                REQUIRE((reader.read(4) == 8));
            }
        }

        SECTION("Bigger test")
        {
            T reader(from_bits(
                {0b10101010, 0b11110000, 0b00110011}));
            const std::vector<u32> actual_value
                = {reader.read(1), reader.read(23)};
            const auto expected_value = type == TestType::Msb
                ? std::vector<u32>{1, 0b01010101111000000110011}
                : std::vector<u32>{0, 0b00110011111100001010101};
            REQUIRE((actual_value == expected_value));
        }

        SECTION("Max bit reader capacity test (unaligned)")
        {
            T reader(from_bits(
                {0b11001100, 0b10101010, 0b11110000, 0b00110011}));
            const auto actual_value = reader.read(32);
            const auto expected_value = type == TestType::Msb
                ? 0b11001100'10101010'11110000'00110011
                : 0b00110011'11110000'10101010'11001100;
            REQUIRE((actual_value == expected_value));
        }

        SECTION("Max bit reader capacity test (aligned)")
        {
            T reader(from_bits(
                {0b11001100, 0b10101010, 0b11110000, 0b00110011, 0b01010101}));
            reader.read(1);
            const auto actual_value = reader.read(32);
            const auto expected_value = type == TestType::Msb
                ? 0b1001100'10101010'11110000'00110011'0
                : 0b1'00110011'11110000'10101010'1100110;
            REQUIRE((actual_value == expected_value));
        }
    }
}

template<class T> static void test_writing(const TestType type)
{
    SECTION("Writing")
    {
        SECTION("Aligned")
        {
            io::MemoryByteStream output_stream;
            {
                T writer(output_stream);
                writer.write(1, 0b1);
                writer.write(7, 0b0111111);
            }
            const auto output = output_stream.seek(0).read_to_eof();
            REQUIRE((output == "\xBF"_b));
        }

        SECTION("Unaligned")
        {
            io::MemoryByteStream output_stream;
            {
                T writer(output_stream);
                writer.write(1, 0b1);
                writer.write(10, 0b1011111101);
            }
            const auto output = output_stream.seek(0).read_to_eof();
            REQUIRE((output == "\xDF\xA0"_b));
        }

        SECTION("Max value")
        {
            io::MemoryByteStream output_stream;
            {
                T writer(output_stream);
                writer.write(32, 0xFFFFFFFF);
            }
            const auto output = output_stream.seek(0).read_to_eof();
            REQUIRE((output == "\xFF\xFF\xFF\xFF"_b));
        }

        SECTION("Values exceeding masks")
        {
            io::MemoryByteStream output_stream;
            {
                T writer(output_stream);
                writer.write(1, 8);
                writer.write(1, 7);
            }
            const auto output = output_stream.seek(0).read_to_eof();
            REQUIRE((output == "\x40"_b));
        }

        // SECTION("Interleaving")
        // {
        //     io::MemoryByteStream output_stream("\xFF\xFF"_b);
        //     output_stream.seek(0);
        //     {
        //         T writer(output_stream);
        //         writer.seek(3);
        //         writer.write(8, 0);
        //     }
        //     const auto output = output_stream.seek(0).read_to_eof();
        //     INFO(output.size());
        //     REQUIRE((output == "\xE0\x1F"_b));
        // }
    }
}

template<class T> static void test_checking_for_eof()
{
    SECTION("Checking for EOF")
    {
        T reader("\x00\x00"_b);
        reader.read(7);
        REQUIRE((reader.left() == 9));
        reader.read(7);
        REQUIRE((reader.left() == 2));
        reader.read(1);
        REQUIRE((reader.left() == 1));
        reader.read(1);
        REQUIRE((reader.left() == 0));
    }
}

template<class T> static void test_checking_size()
{
    SECTION("Checking size")
    {
        T reader1("\x00\x00"_b);
        REQUIRE((reader1.size() == 16));
        T reader2("\x00"_b);
        REQUIRE((reader2.size() == 8));
        T reader3(""_b);
        REQUIRE((reader3.size() == 0));
    }
}

template<class T> static void test_seeking()
{
    SECTION("Seeking")
    {
        SECTION("Integer aligned")
        {
            T reader(from_bits({
                0b00000000, 0b00000000, 0b00000000, 0b00000000,
                0b11111111, 0b11111111, 0b11111111, 0b11111111,
                0b11001100, 0b10101010, 0b11110000, 0b00110011}));
            reader.seek(0);
            REQUIRE((reader.read(32) == 0b00000000000000000000000000000000));
            reader.seek(32);
            REQUIRE((reader.read(32) == 0b11111111111111111111111111111111));
            reader.seek(64);
            REQUIRE((reader.read(32) == 0b11001100101010101111000000110011));
        }

        SECTION("Byte aligned")
        {
            T reader(from_bits(
                {0b11001100, 0b10101010, 0b11110000, 0b00110011}));
            reader.seek(0);  REQUIRE((reader.read(8) == 0b11001100));
            reader.seek(8);  REQUIRE((reader.read(8) == 0b10101010));
            reader.seek(16); REQUIRE((reader.read(8) == 0b11110000));
            reader.seek(24); REQUIRE((reader.read(8) == 0b00110011));
        }

        SECTION("Unaligned")
        {
            T reader(from_bits(
                {0b11001100, 0b10101010, 0b11110000, 0b00110011}));
            reader.seek(0);  REQUIRE((reader.read(8) == 0b11001100));
            reader.seek(1);  REQUIRE((reader.read(8) == 0b10011001));
            reader.seek(2);  REQUIRE((reader.read(8) == 0b00110010));
            reader.seek(3);  REQUIRE((reader.read(8) == 0b01100101));
            reader.seek(4);  REQUIRE((reader.read(8) == 0b11001010));
            reader.seek(5);  REQUIRE((reader.read(8) == 0b10010101));
            reader.seek(6);  REQUIRE((reader.read(8) == 0b00101010));
            reader.seek(7);  REQUIRE((reader.read(8) == 0b01010101));
            reader.seek(8);  REQUIRE((reader.read(8) == 0b10101010));
            reader.seek(9);  REQUIRE((reader.read(8) == 0b01010101));
            reader.seek(10); REQUIRE((reader.read(8) == 0b10101011));
            reader.seek(11); REQUIRE((reader.read(8) == 0b01010111));
            reader.seek(12); REQUIRE((reader.read(8) == 0b10101111));
            reader.seek(13); REQUIRE((reader.read(8) == 0b01011110));
            reader.seek(14); REQUIRE((reader.read(8) == 0b10111100));
            reader.seek(15); REQUIRE((reader.read(8) == 0b01111000));
            reader.seek(16); REQUIRE((reader.read(8) == 0b11110000));
            reader.seek(17); REQUIRE((reader.read(8) == 0b11100000));
            reader.seek(18); REQUIRE((reader.read(8) == 0b11000000));
            reader.seek(19); REQUIRE((reader.read(8) == 0b10000001));
            reader.seek(20); REQUIRE((reader.read(8) == 0b00000011));
            reader.seek(21); REQUIRE((reader.read(8) == 0b00000110));
            reader.seek(22); REQUIRE((reader.read(8) == 0b00001100));
            reader.seek(23); REQUIRE((reader.read(8) == 0b00011001));
            reader.seek(24); REQUIRE((reader.read(8) == 0b00110011));
        }

        SECTION("Unaligned (automatic)")
        {
            T reader(from_bits(
                {0b11001100, 0b10101010, 0b11110000, 0b00110011}));
            for (const auto i : algo::range(32))
            {
                reader.seek(i);
                INFO("Position: " << reader.pos());
                auto mask = (1ull << (32 - i)) - 1;
                auto expected = 0b11001100101010101111000000110011ull & mask;
                REQUIRE((reader.read(32 - i) == expected));
            }
        }

        SECTION("Seeking beyond EOF throws errors")
        {
            T reader(from_bits(
                {0b11001100, 0b10101010, 0b11110000, 0b00110011}));
            for (const auto i : algo::range(32))
            {
                reader.seek(31);
                REQUIRE_THROWS(reader.skip(2 + i));
                REQUIRE((reader.pos() == 31));
            }
            for (const auto i : algo::range(32))
                REQUIRE_THROWS(reader.seek(33 + i));
        }
    }

    SECTION("Skipping")
    {
        T reader(from_bits(
            {0b11001100, 0b10101010, 0b11110000, 0b00110011}));
        reader.seek(0);
        REQUIRE((reader.read(8) == 0b11001100));
        reader.skip(-7);
        REQUIRE((reader.read(8) == 0b10011001));
    }
}

template<class T> static void test_stream_interop()
{
    SECTION("Stream interop")
    {
        SECTION("Aligned")
        {
            io::MemoryByteStream stream("\xFF\x01"_b);
            T reader(stream);
            REQUIRE((reader.read(8) == 0xFF));
            REQUIRE((stream.read<u8>() == 1));
        }

        SECTION("Unaligned")
        {
            io::MemoryByteStream stream("\xFF\x80\x02"_b);
            T reader(stream);
            REQUIRE((reader.read(8) == 0xFF));
            REQUIRE((reader.read(1) == 0x01));
            REQUIRE((stream.read<u8>() == 2));
        }

        SECTION("Interleaving")
        {
            io::MemoryByteStream stream("\xFF\xC0\x02\x01\xFF"_b);
            T reader(stream);
            REQUIRE((reader.read(8) == 0xFF));
            REQUIRE((reader.read(1) == 0x01));
            REQUIRE((stream.read<u8>() == 2));
            REQUIRE((reader.read(7) == 0x40));
            REQUIRE((stream.read<u8>() == 1));
            REQUIRE((reader.read(8) == 0xFF));
        }

        SECTION("Interleaving with seeking")
        {
            io::MemoryByteStream stream("\xFF\xC0\x02\x01\xFF"_b);
            T reader(stream);
            REQUIRE((reader.read(8) == 0xFF));
            REQUIRE((reader.read(1) == 0x01));
            REQUIRE((stream.read<u8>() == 2));
            reader.seek(stream.pos() << 3);
            REQUIRE((reader.read(8) == 1));
            REQUIRE((stream.read<u8>() == 0xFF));
        }
    }
}

template<class T> static void test_retracting()
{
    SECTION("Reading beyond EOF retracts to prior offset")
    {
        SECTION("Byte-aligned without byte retrieval")
        {
            T reader("\x00"_b);
            reader.read(7);
            reader.read(1);
            REQUIRE((reader.left() == 0));
            REQUIRE_THROWS(reader.read(1));
            REQUIRE((reader.left() == 0));
            REQUIRE((reader.pos() == 8));
        }

        SECTION("Byte-aligned with byte retrieval")
        {
            T reader("\x00\xFF"_b);
            reader.read(7);
            reader.read(1);
            REQUIRE_THROWS(reader.read(16));
            REQUIRE((reader.pos() == 8));
            REQUIRE((reader.read(8) == 0xFF));
        }

        SECTION("Byte-unaligned without byte retrieval")
        {
            T reader("\x01"_b);
            reader.read(7);
            REQUIRE_THROWS(reader.read(2));
            REQUIRE((reader.pos() == 7));
            REQUIRE(reader.read(1));
        }

        SECTION("Byte-unaligned with byte retrieval")
        {
            T reader("\x01\x00"_b);
            reader.read(7);
            REQUIRE_THROWS(reader.read(10));
            REQUIRE((reader.pos() == 7));
            REQUIRE(reader.read(1));
        }
    }
}

TEST_CASE("BaseBitStream", "[io]")
{
    test_reading_missing_bits<io::MsbBitStream>();
    test_checking_for_eof<io::MsbBitStream>();
    test_checking_size<io::MsbBitStream>();
    test_seeking<io::MsbBitStream>();
    test_stream_interop<io::MsbBitStream>();
    test_retracting<io::MsbBitStream>();
}

TEST_CASE("LsbBitStream", "[io]")
{
    test_reading_single_bits<io::LsbBitStream>(TestType::Lsb);
    test_reading_multiple_bits<io::LsbBitStream>(TestType::Lsb);
    test_reading_multiple_bytes<io::LsbBitStream>(TestType::Lsb);
}

TEST_CASE("MsbBitStream", "[io]")
{
    test_reading_single_bits<io::MsbBitStream>(TestType::Msb);
    test_reading_multiple_bits<io::MsbBitStream>(TestType::Msb);
    test_reading_multiple_bytes<io::MsbBitStream>(TestType::Msb);
    test_writing<io::MsbBitStream>(TestType::Msb);
}
