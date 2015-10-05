#include "test_support/archive_support.h"
#include "file.h"
#include "test_support/catch.hh"

using namespace au;

std::vector<std::shared_ptr<File>> tests::unpack_to_memory(
    const boost::filesystem::path &input_path, fmt::ArchiveDecoder &decoder)
{
    File file(input_path, io::FileMode::Read);
    return decoder.unpack(file);
}
