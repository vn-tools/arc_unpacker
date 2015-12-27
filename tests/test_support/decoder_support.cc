#include "test_support/decoder_support.h"
#include "test_support/catch.h"

using namespace au;

// This is to test whether ImageDecoder::decode, IDecoder::is_recognized etc.
// take care of stream position themselves rather than relying on the callers.
static void navigate_to_random_place(io::Stream &stream)
{
    if (stream.size())
        stream.seek(rand() % stream.size());
}

std::vector<std::shared_ptr<io::File>> tests::unpack(
    const fmt::BaseArchiveDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    const auto meta = decoder.read_meta(dummy_logger, input_file);
    std::vector<std::shared_ptr<io::File>> files;
    for (const auto &entry : meta->entries)
    {
        files.push_back(decoder.read_file(
            dummy_logger, input_file, *meta, *entry));
    }
    return files;
}

std::unique_ptr<io::File> tests::decode(
    const fmt::BaseFileDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    return decoder.decode(dummy_logger, input_file);
}

res::Image tests::decode(
    const fmt::BaseImageDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    return decoder.decode(dummy_logger, input_file);
}

res::Audio tests::decode(
    const fmt::BaseAudioDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    return decoder.decode(dummy_logger, input_file);
}
