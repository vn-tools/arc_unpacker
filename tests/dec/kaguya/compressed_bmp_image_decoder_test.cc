#include "dec/kaguya/compressed_bmp_image_decoder.h"
#include "algo/pack/lzss.h"
#include "enc/microsoft/bmp_image_encoder.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

namespace
{
    class CustomLzssWriter final : public algo::pack::BaseLzssWriter
    {
    public:
        CustomLzssWriter(const size_t reserve_size);
        void write_literal(const u8 literal) override;
        void write_repetition(
            const size_t position_bits,
            const size_t position,
            const size_t size_bits,
            const size_t size) override;
        bstr retrieve() override;

    private:
        size_t control_count, control_pos;
        size_t repeat_count, repeat_pos;
        bstr output;
    };
}

CustomLzssWriter::CustomLzssWriter(const size_t reserve_size)
    : control_count(0), repeat_count(0)
{
    output.reserve(reserve_size);
}

void CustomLzssWriter::write_literal(const u8 literal)
{
    if (control_count == 0)
    {
        output += "\x00"_b;
        control_pos = output.size() - 1;
        control_count = 8;
    }
    output[control_pos] <<= 1;
    output[control_pos] |= 1;
    control_count--;
    output += literal;
}

void CustomLzssWriter::write_repetition(
    const size_t position_bits,
    const size_t position,
    const size_t size_bits,
    const size_t size)
{
    if (control_count == 0)
    {
        output += "\x00"_b;
        control_pos = output.size() - 1;
        control_count = 8;
    }
    output[control_pos] <<= 1;
    control_count--;
    output += static_cast<u8>(position);
    if (repeat_count == 0)
    {
        output += "\x00"_b;
        repeat_pos = output.size() - 1;
        repeat_count = 8;
    }
    output[repeat_pos] >>= 4;
    output[repeat_pos] |= size << 4;
    repeat_count -= 4;
}

bstr CustomLzssWriter::retrieve()
{
    return output;
}

static bstr compress(const bstr &input)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 8;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 0xEF;
    CustomLzssWriter writer(input.size());
    io::MemoryStream input_stream(input);
    return algo::pack::lzss_compress(input_stream, settings, writer);
}

TEST_CASE("Kaguya compressed BMP images", "[dec]")
{
    const auto expected_image = tests::get_transparent_test_image();

    Logger dummy_logger;
    dummy_logger.mute();
    const auto bmp_encoder = enc::microsoft::BmpImageEncoder();
    const auto bmp_file = bmp_encoder.encode(
        dummy_logger, expected_image, "test.bmp");

    const auto data_orig = bmp_file->stream.seek(0).read_to_eof();
    const auto data_comp = compress(data_orig);

    io::File input_file;
    input_file.stream.write_le<u32>(data_orig.size());
    input_file.stream.write(data_comp);

    const auto decoder = CompressedBmpImageDecoder();
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(expected_image, actual_image);
}
