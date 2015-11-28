#include "io/memory_stream.h"
#include "test_support/catch.hh"
#include "test_support/stream_test.h"

using namespace au;
using namespace au::io;

TEST_CASE("MemoryStream", "[io][stream]")
{
    tests::stream_test(
        []()
        {
            return std::make_unique<MemoryStream>();
        },
        []()
        {
        });
}
