#include "algo/ptr.h"
#include "test_support/catch.h"
#include "types.h"

using namespace au;
using namespace au::algo;

TEST_CASE("ptr", "[core]")
{
    SECTION("Constructor")
    {
        std::vector<u32> data = {1, 2, 3};
        auto p = ptr<u32>(data.data(), data.size());
        REQUIRE(p.size() == 3);
    }

    SECTION("make_ptr")
    {
        const bstr data = "herp"_b;
        const auto p = make_ptr(data);
        REQUIRE(p.size() == 4);

        const auto p2 = make_ptr(data.get<const u8>(), 3);
        REQUIRE(p2.size() == 3);
    }

    SECTION("Accessing the elements")
    {
        const bstr data = "herp"_b;
        auto p = make_ptr(data);
        REQUIRE(p[0] == 'h');
        REQUIRE(p[1] == 'e');
        REQUIRE(*p.start() == 'h');
        REQUIRE(*p == 'h');
        REQUIRE(p.end()[-1] == 'p');
    }

    SECTION("Navigating")
    {
        const bstr data = "herp"_b;
        auto p = make_ptr(data);
        p++;
        REQUIRE(p[0] == 'e');
        p--;
        REQUIRE(p[0] == 'h');
        p += 2;
        REQUIRE(p[0] == 'r');
        p -= 2;
        REQUIRE(p[0] == 'h');
    }

    SECTION("Modifying and navigating at once")
    {
        bstr data = "herp"_b;
        auto p = make_ptr(data);
        {
            *p++ = 'H';
            *p++ = 'E';
        }
        REQUIRE(data == "HErp"_b);
    }

    SECTION("No problems with strict aliasing")
    {
        bstr data = "herp"_b;
        auto p = make_ptr(data);
        *reinterpret_cast<int*>(&p[0]) = 0x12345678;
        REQUIRE(data == "\x78\x56\x34\x12"_b);
    }
}
