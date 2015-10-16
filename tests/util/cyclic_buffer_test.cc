#include "test_support/catch.hh"
#include "util/cyclic_buffer.h"
#include "util/range.h"

using namespace au;
using namespace au::util;

TEST_CASE("Empty cyclic buffer content", "[util]")
{
    CyclicBuffer buffer(5, 1);
    for (auto i : util::range(5))
        REQUIRE(!buffer[i]);
}

TEST_CASE("Cyclic buffer size", "[util]")
{
    CyclicBuffer buffer(5, 1);
    REQUIRE(buffer.size() == 5);
    REQUIRE(buffer.pos() == 1);
}

TEST_CASE("Cyclic buffer byte pushing", "[util]")
{
    CyclicBuffer buffer(5, 1);
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

TEST_CASE("Cyclic buffer string pushing", "[util]")
{
    CyclicBuffer buffer(5, 1);
    SECTION("Small strings")
    {
        buffer << "123"_b;
        REQUIRE(buffer[1] == '1');
        REQUIRE(buffer[2] == '2');
        REQUIRE(buffer[3] == '3');
        REQUIRE(buffer.pos() == 4);
    }
    SECTION("Medium strings (one cycle)")
    {
        buffer << "12345"_b;
        REQUIRE(buffer[1] == '1');
        REQUIRE(buffer[2] == '2');
        REQUIRE(buffer[3] == '3');
        REQUIRE(buffer[4] == '4');
        REQUIRE(buffer[0] == '5');
        REQUIRE(buffer.pos() == 1);
    }
    SECTION("Longer strings (two cycle)")
    {
        buffer << "1234567890"_b;
        REQUIRE(buffer[1] == '6');
        REQUIRE(buffer[2] == '7');
        REQUIRE(buffer[3] == '8');
        REQUIRE(buffer[4] == '9');
        REQUIRE(buffer[0] == '0');
        REQUIRE(buffer.pos() == 1);
    }
}

TEST_CASE("Cyclic buffer start position", "[util]")
{
    SECTION("Zero start position, strings")
    {
        CyclicBuffer buffer(5, 0);
        REQUIRE(buffer.start() == 0);
        buffer << "123"_b;
        REQUIRE(buffer.start() == 0);
        buffer << "45"_b;
        REQUIRE(buffer.start() == 0);
        buffer << "6"_b;
        REQUIRE(buffer.start() == 1);
        buffer << "7"_b;
        REQUIRE(buffer.start() == 2);
        buffer << "89"_b;
        REQUIRE(buffer.start() == 4);
        buffer << "0"_b;
        REQUIRE(buffer.start() == 0);
    }
    SECTION("Custom start position, strings")
    {
        CyclicBuffer buffer(5, 1);
        REQUIRE(buffer.start() == 1);
        buffer << "123"_b;
        REQUIRE(buffer.start() == 1);
        buffer << "45"_b;
        REQUIRE(buffer.start() == 1);
        buffer << "6"_b;
        REQUIRE(buffer.start() == 2);
        buffer << "7"_b;
        REQUIRE(buffer.start() == 3);
        buffer << "89"_b;
        REQUIRE(buffer.start() == 0);
        buffer << "0"_b;
        REQUIRE(buffer.start() == 1);
    }
    SECTION("Zero start position bytes")
    {
        CyclicBuffer buffer(5, 0);
        REQUIRE(buffer.start() == 0);
        buffer << '1';
        REQUIRE(buffer.start() == 0);
        buffer << '2';
        REQUIRE(buffer.start() == 0);
        buffer << '3';
        REQUIRE(buffer.start() == 0);
        buffer << '4';
        REQUIRE(buffer.start() == 0);
        buffer << '5';
        REQUIRE(buffer.start() == 0);
        buffer << '6';
        REQUIRE(buffer.start() == 1);
        buffer << '7';
        REQUIRE(buffer.start() == 2);
        buffer << '8';
        REQUIRE(buffer.start() == 3);
        buffer << '9';
        REQUIRE(buffer.start() == 4);
        buffer << '0';
        REQUIRE(buffer.start() == 0);
    }
    SECTION("Custom start position bytes")
    {
        CyclicBuffer buffer(5, 1);
        REQUIRE(buffer.start() == 1);
        buffer << '1';
        REQUIRE(buffer.start() == 1);
        buffer << '2';
        REQUIRE(buffer.start() == 1);
        buffer << '3';
        REQUIRE(buffer.start() == 1);
        buffer << '4';
        REQUIRE(buffer.start() == 0);
        buffer << '5';
        REQUIRE(buffer.start() == 1);
        buffer << '6';
        REQUIRE(buffer.start() == 2);
        buffer << '7';
        REQUIRE(buffer.start() == 3);
        buffer << '8';
        REQUIRE(buffer.start() == 4);
        buffer << '9';
        REQUIRE(buffer.start() == 0);
        buffer << '0';
        REQUIRE(buffer.start() == 1);
    }
}

TEST_CASE("Cyclic buffer manual overwriting", "[util]")
{
    CyclicBuffer buffer(2, 0);
    buffer << "1"_b;
    buffer[0] = '2';
    REQUIRE(buffer[0] == '2');
    REQUIRE(!buffer[1]);
}

TEST_CASE("Cyclic buffer access out of bounds", "[util]")
{
    CyclicBuffer buffer(2, 0);
    buffer[0] = '0';
    buffer[1] = '1';
    REQUIRE(buffer[2] == '0');
    REQUIRE(buffer[3] == '1');
}
