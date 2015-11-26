#include "test_support/decoder_support.h"
#include "test_support/catch.hh"

using namespace au;

// This is to test whether ImageDecoder::decode, IDecoder::is_recognized etc.
// take care of stream position themselves rather than relying on the callers.
static void navigate_to_random_place(io::Stream &stream)
{
    if (stream.size())
        stream.seek(rand() % stream.size());
}

std::vector<std::shared_ptr<io::File>> tests::unpack(
    const fmt::ArchiveDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    const auto meta = decoder.read_meta(input_file);
    std::vector<std::shared_ptr<io::File>> files;
    for (const auto &entry : meta->entries)
        files.push_back(decoder.read_file(input_file, *meta, *entry));
    return files;
}

std::unique_ptr<io::File> tests::decode(
    const fmt::FileDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    return decoder.decode(input_file);
}

pix::Grid tests::decode(
    const fmt::ImageDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    return decoder.decode(input_file);
}

sfx::Audio tests::decode(
    const fmt::AudioDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    return decoder.decode(input_file);
}
