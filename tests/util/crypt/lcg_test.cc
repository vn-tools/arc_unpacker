#include "util/crypt/lcg.h"
#include "test_support/catch.hh"

using namespace au::util::crypt;

TEST_CASE(
    "Linear congruential generators work (Microsoft Visual C++)",
    "[util][crypt]")
{
    Lcg l(LcgKind::MicrosoftVisualC, 0xDEADBEEF);
    REQUIRE(l.next() == 18337);
    REQUIRE(l.next() == 16920);
    REQUIRE(l.next() == 18732);
    REQUIRE(l.next() == 19770);
    REQUIRE(l.next() == 16163);
    REQUIRE(l.next() == 20831);
    REQUIRE(l.next() == 31795);
    REQUIRE(l.next() == 19149);
}

TEST_CASE("Linear congruential generators work (Park&Miller)", "[util][crypt]")
{
    Lcg l(LcgKind::ParkMiller, 0xDEADBEEF);
    REQUIRE(l.next() == 193);
    REQUIRE(l.next() == 79);
    REQUIRE(l.next() == 133);
    REQUIRE(l.next() == 113);
    REQUIRE(l.next() == 140);
    REQUIRE(l.next() == 117);
    REQUIRE(l.next() == 67);
    REQUIRE(l.next() == 114);
}

TEST_CASE(
    "Linear congruential generators work (Park&Miller revised)",
    "[util][crypt]")
{
    Lcg l(LcgKind::ParkMillerRevised, 0xDEADBEEF);
    REQUIRE(l.next() == 246);
    REQUIRE(l.next() == 50);
    REQUIRE(l.next() == 60);
    REQUIRE(l.next() == 141);
    REQUIRE(l.next() == 213);
    REQUIRE(l.next() == 234);
    REQUIRE(l.next() == 179);
    REQUIRE(l.next() == 38);
}
