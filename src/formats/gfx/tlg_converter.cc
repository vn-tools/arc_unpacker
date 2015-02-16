// TLG image
//
// Company:   -
// Engine:    Kirikiri
// Extension: .tlg
// Archives:  XP3
//
// Known Games:
// - Fate/Stay Night
// - Fate/Hollow Ataraxia

#include <stdexcept>
#include "formats/gfx/tlg_converter.h"
#include "formats/gfx/tlg_converter/tlg5_decoder.h"
#include "formats/gfx/tlg_converter/tlg6_decoder.h"

namespace
{
    const std::string magic_tlg_0("TLG0.0\x00sds\x1a", 11);
    const std::string magic_tlg_5("TLG5.0\x00raw\x1a", 11);
    const std::string magic_tlg_6("TLG6.0\x00raw\x1a", 11);

    void guess_version_and_decode(File &file);

    std::string extract_string(std::string &container)
    {
        size_t length = 0;
        while (container.length() && container[0] >= '0' && container[0] <= '9')
        {
            length *= 10;
            length += container[0] - '0';
            container.erase(0, 1);
        }
        container.erase(0, 1);
        std::string str = container.substr(0, length);
        container.erase(0, length + 1);
        return str;
    }

    void decode_tlg_0(File &file)
    {
        size_t raw_data_size = file.io.read_u32_le();
        size_t raw_data_offset = file.io.tell();

        std::vector<std::pair<std::string, std::string>> tags;

        file.io.skip(raw_data_size);
        while (file.io.tell() < file.io.size())
        {
            std::string chunk_name = file.io.read(4);
            size_t chunk_size = file.io.read_u32_le();
            std::string chunk_data = file.io.read(chunk_size);

            if (chunk_name == "tags")
            {
                while (chunk_data != "")
                {
                    std::string key = extract_string(chunk_data);
                    std::string value = extract_string(chunk_data);
                    tags.push_back(
                        std::pair<std::string, std::string>(key, value));
                }
            }
            else
                throw std::runtime_error("Unknown chunk: " + chunk_name);
        }

        file.io.seek(raw_data_offset);
        guess_version_and_decode(file);
    }

    void decode_tlg_5(File &file)
    {
        Tlg5Decoder decoder;
        decoder.decode(file);
    }

    void decode_tlg_6(File &file)
    {
        Tlg6Decoder decoder;
        decoder.decode(file);
    }

    void guess_version_and_decode(File &file)
    {
        size_t pos = file.io.tell();

        file.io.seek(pos);
        if (file.io.read(magic_tlg_0.size()) ==  magic_tlg_0)
        {
            decode_tlg_0(file);
            return;
        }

        file.io.seek(pos);
        if (file.io.read(magic_tlg_5.size()) ==  magic_tlg_5)
        {
            decode_tlg_5(file);
            return;
        }

        file.io.seek(pos);
        if (file.io.read(magic_tlg_6.size()) ==  magic_tlg_6)
        {
            decode_tlg_6(file);
            return;
        }

        throw std::runtime_error("Not a TLG image");
    }
}

void TlgConverter::decode_internal(File &file) const
{
    guess_version_and_decode(file);
}
