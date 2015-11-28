#include "test_support/catch.hh"
#include "util/call_stack_keeper.h"

using namespace au;
using namespace au::util;

TEST_CASE("Call stack keeper", "[util]")
{
    SECTION("The action is invoked at all")
    {
        CallStackKeeper keeper(3);
        bool executed = false;
        keeper.recurse([&]() { executed = true; });
        REQUIRE(executed);
    }

    SECTION("Breaking recursion limit throws an exception")
    {
        CallStackKeeper keeper(3);
        keeper.recurse([&]()
            {
                keeper.recurse([&]()
                    {
                        keeper.recurse([&]()
                            {
                                REQUIRE_THROWS(keeper.recurse([]() { }));
                            });

                        REQUIRE_NOTHROW(keeper.recurse([]() { }));

                        keeper.recurse([&]()
                            {
                                REQUIRE_THROWS(keeper.recurse([]() { }));
                            });
                    });
            });
    }

    SECTION("Checking for recursion limit")
    {
        CallStackKeeper keeper(2);
        REQUIRE(!keeper.recursion_limit_reached());

        keeper.recurse([&]()
            {
                REQUIRE(!keeper.recursion_limit_reached());

                keeper.recurse([&]()
                    {
                        REQUIRE(keeper.recursion_limit_reached());
                    });

                REQUIRE(!keeper.recursion_limit_reached());
            });
    }

    SECTION("Handling exceptions")
    {
        CallStackKeeper keeper(1);
        REQUIRE(!keeper.recursion_limit_reached());
        REQUIRE_THROWS(keeper.recurse([&]()
            {
                REQUIRE(keeper.recursion_limit_reached());
                throw std::logic_error("!");
            }));
        REQUIRE(!keeper.recursion_limit_reached());
    }
}
