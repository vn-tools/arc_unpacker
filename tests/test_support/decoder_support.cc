#include "test_support/decoder_support.h"
#include "file.h"
#include "test_support/catch.hh"

using namespace au;

// This is to test whether ImageDecoder::decode, IDecoder::is_recognized etc.
// take care of stream position themselves rather than relying on the callers.
static void navigate_to_random_place(io::IO &io)
{
    if (io.size())
        io.seek(rand() % io.size());
}

std::vector<std::shared_ptr<File>> tests::unpack(
    const fmt::ArchiveDecoder &decoder, File &input_file)
{
    navigate_to_random_place(input_file.io);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.io);
    auto meta = decoder.read_meta(input_file);
    std::vector<std::shared_ptr<File>> files;
    for (auto &entry : meta->entries)
        files.push_back(decoder.read_file(input_file, *meta, *entry));
    return files;
}

std::unique_ptr<File> tests::decode(
    const fmt::FileDecoder &decoder, File &input_file)
{
    navigate_to_random_place(input_file.io);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.io);
    return decoder.decode(input_file);
}

pix::Grid tests::decode(const fmt::ImageDecoder &decoder, File &input_file)
{
    navigate_to_random_place(input_file.io);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.io);
    return decoder.decode(input_file);
}
