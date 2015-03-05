#include "util/lzss.h"

std::string lzss_decompress(
    const std::string &input,
    size_t size_original,
    const LzssSettings &settings)
{
    BitReader bit_reader(input.data(), input.size());
    return lzss_decompress(bit_reader, size_original, settings);
}

std::string lzss_decompress(
    BitReader &bit_reader,
    size_t size_original,
    const LzssSettings &settings)
{
    std::string output;
    size_t dictionary_size = 1 << settings.position_bits;
    size_t dictionary_pos = settings.initial_dictionary_pos;
    std::unique_ptr<unsigned char> dictionary(
        new unsigned char[dictionary_size]);

    unsigned char *dictionary_ptr = dictionary.get();

    while (output.size() < size_original)
    {
        if (bit_reader.get(1) > 0)
        {
            unsigned char byte = bit_reader.get(8);
            output.push_back(byte);
            dictionary_ptr[dictionary_pos] = byte;
            dictionary_pos ++;
            dictionary_pos %= dictionary_size;
        }
        else
        {
            unsigned int pos = bit_reader.get(settings.position_bits);
            unsigned int length = bit_reader.get(settings.length_bits);
            length += settings.min_match_length;
            for (size_t i = 0; i < length; i ++)
            {
                unsigned char byte = dictionary_ptr[pos];
                pos += 1;
                pos %= dictionary_size;

                if (settings.reuse_compressed)
                {
                    dictionary_ptr[dictionary_pos] = byte;
                    dictionary_pos ++;
                    dictionary_pos %= dictionary_size;
                }
                output.push_back(byte);
                if (output.size() >= size_original)
                    break;
            }
        }
    }

    return output;
}
