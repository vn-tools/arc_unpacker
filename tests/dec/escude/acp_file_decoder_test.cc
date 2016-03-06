#include "dec/escude/acp_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::escude;

static const io::path dir = "tests/dec/escude/files/acp/";

TEST_CASE("Escude ACP files", "[dec]")
{
    const auto decoder = AcpFileDecoder();
    const auto input_file = tests::file_from_path(dir / "misa01.BMP");
    const auto expected_file = tests::file_from_path(dir / "misa01-out.BMP");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}
