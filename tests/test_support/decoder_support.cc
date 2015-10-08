#include "test_support/decoder_support.h"
#include "file.h"
#include "test_support/catch.hh"

using namespace au;

std::vector<std::shared_ptr<File>> tests::unpack(
    const fmt::ArchiveDecoder &decoder, File &input_file)
{
    return decoder.unpack(input_file);
}

std::unique_ptr<File> tests::decode(
    const fmt::FileDecoder &decoder, File &input_file)
{
    return decoder.decode(input_file);
}

pix::Grid tests::decode(const fmt::ImageDecoder &decoder, File &input_file)
{
    return decoder.decode(input_file);
}
