#include "util/call_stack_keeper.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Call stack keeper", "[util]")
{
    SECTION("The action is invoked at all")
    {
        util::CallStackKeeper keeper(3);
        bool executed = false;
        keeper.recurse([&]() { executed = true; });
        REQUIRE(executed);
    }

    SECTION("Breaking recursion limit throws an exception")
    {
        util::CallStackKeeper keeper(3);
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
        util::CallStackKeeper keeper(2);
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
        util::CallStackKeeper keeper(1);
        REQUIRE(!keeper.recursion_limit_reached());
        REQUIRE_THROWS(keeper.recurse([&]()
            {
                REQUIRE(keeper.recursion_limit_reached());
                throw std::logic_error("!");
            }));
        REQUIRE(!keeper.recursion_limit_reached());
    }
}
