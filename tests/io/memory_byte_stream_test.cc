#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/stream_test.h"

using namespace au;

TEST_CASE("MemoryByteStream", "[io][stream]")
{
    SECTION("Full test suite")
    {
        tests::stream_test(
            []() { return std::make_unique<io::MemoryByteStream>(); },
            []() { });
    }
}
