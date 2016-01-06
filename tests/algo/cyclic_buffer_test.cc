#include "algo/cyclic_buffer.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "types.h"

using namespace au;

TEST_CASE("CyclicBuffer", "[algo]")
{
    SECTION("Empty buffer content")
    {
        const algo::CyclicBuffer<u8, 5> buffer(1);
        for (auto i : algo::range(5))
            REQUIRE(!buffer[i]);
    }

    SECTION("Size")
    {
        const algo::CyclicBuffer<u8, 5> buffer(1);
        REQUIRE(buffer.size() == 5);
        REQUIRE(buffer.pos() == 1);
    }

    SECTION("Pushing single bytes")
    {
        algo::CyclicBuffer<u8, 5> buffer(1);
        SECTION("Small strings")
        {
            buffer << '1';
            buffer << '2';
            buffer << '3';
            REQUIRE(buffer[1] == '1');
            REQUIRE(buffer[2] == '2');
            REQUIRE(buffer[3] == '3');
            REQUIRE(buffer.pos() == 4);
        }
        SECTION("Medium strings (one cycle)")
        {
            buffer << '1';
            buffer << '2';
            buffer << '3';
            buffer << '4';
            buffer << '5';
            REQUIRE(buffer[1] == '1');
            REQUIRE(buffer[2] == '2');
            REQUIRE(buffer[3] == '3');
            REQUIRE(buffer[4] == '4');
            REQUIRE(buffer[0] == '5');
            REQUIRE(buffer.pos() == 1);
        }
        SECTION("Longer strings (two cycle)")
        {
            buffer << '1';
            buffer << '2';
            buffer << '3';
            buffer << '4';
            buffer << '5';
            buffer << '6';
            buffer << '7';
            buffer << '8';
            buffer << '9';
            buffer << '0';
            REQUIRE(buffer[1] == '6');
            REQUIRE(buffer[2] == '7');
            REQUIRE(buffer[3] == '8');
            REQUIRE(buffer[4] == '9');
            REQUIRE(buffer[0] == '0');
            REQUIRE(buffer.pos() == 1);
        }
    }

    SECTION("Start position")
    {
        SECTION("Zero start position")
        {
            algo::CyclicBuffer<u8, 5> buffer(0);
            REQUIRE(buffer.start() == 0);
            buffer << '1'; REQUIRE(buffer.start() == 0);
            buffer << '2'; REQUIRE(buffer.start() == 0);
            buffer << '3'; REQUIRE(buffer.start() == 0);
            buffer << '4'; REQUIRE(buffer.start() == 0);
            buffer << '5'; REQUIRE(buffer.start() == 0);
            buffer << '6'; REQUIRE(buffer.start() == 1);
            buffer << '7'; REQUIRE(buffer.start() == 2);
            buffer << '8'; REQUIRE(buffer.start() == 3);
            buffer << '9'; REQUIRE(buffer.start() == 4);
            buffer << '0'; REQUIRE(buffer.start() == 0);
        }
        SECTION("Custom start position")
        {
            algo::CyclicBuffer<u8, 5> buffer(1);
            REQUIRE(buffer.start() == 1);
            buffer << '1'; REQUIRE(buffer.start() == 1);
            buffer << '2'; REQUIRE(buffer.start() == 1);
            buffer << '3'; REQUIRE(buffer.start() == 1);
            buffer << '4'; REQUIRE(buffer.start() == 0);
            buffer << '5'; REQUIRE(buffer.start() == 1);
            buffer << '6'; REQUIRE(buffer.start() == 2);
            buffer << '7'; REQUIRE(buffer.start() == 3);
            buffer << '8'; REQUIRE(buffer.start() == 4);
            buffer << '9'; REQUIRE(buffer.start() == 0);
            buffer << '0'; REQUIRE(buffer.start() == 1);
        }
    }

    SECTION("Manual overwriting")
    {
        algo::CyclicBuffer<u8, 2> buffer(0);
        buffer << '1';
        buffer[0] = '2';
        REQUIRE(buffer[0] == '2');
        REQUIRE(!buffer[1]);
    }

    SECTION("Access out of bounds")
    {
        algo::CyclicBuffer<u8, 2> buffer(0);
        buffer[0] = '0';
        buffer[1] = '1';
        REQUIRE(buffer[2] == '0');
        REQUIRE(buffer[3] == '1');
    }
}
