#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;

std::string au::util::pack::lzss_decompress(
    const std::string &input, size_t orig_size, const LzssSettings &settings)
{
    io::BitReader bit_reader(input.data(), input.size());
    return lzss_decompress(bit_reader, orig_size, settings);
}

std::string au::util::pack::lzss_decompress(
    io::BitReader &bit_reader, size_t orig_size, const LzssSettings &settings)
{
    std::string output;
    size_t dictionary_size = 1 << settings.position_bits;
    size_t dictionary_pos = settings.initial_dictionary_pos;
    std::unique_ptr<u8[]> dictionary(new u8[dictionary_size]);

    u8 *dictionary_ptr = dictionary.get();

    while (output.size() < orig_size)
    {
        if (bit_reader.get(1) > 0)
        {
            u8 byte = bit_reader.get(8);
            output.push_back(byte);
            dictionary_ptr[dictionary_pos] = byte;
            dictionary_pos++;
            dictionary_pos %= dictionary_size;
        }
        else
        {
            unsigned int pos = bit_reader.get(settings.position_bits);
            unsigned int length = bit_reader.get(settings.length_bits);
            length += settings.min_match_length;
            for (auto i : util::range(length))
            {
                u8 byte = dictionary_ptr[pos];
                pos += 1;
                pos %= dictionary_size;

                if (settings.reuse_compressed)
                {
                    dictionary_ptr[dictionary_pos] = byte;
                    dictionary_pos++;
                    dictionary_pos %= dictionary_size;
                }
                output.push_back(byte);
                if (output.size() >= orig_size)
                    break;
            }
        }
    }

    return output;
}
