// TLG image
//
// Company:   -
// Engine:    Kirikiri
// Extension: .tlg
// Archives:  XP3
//
// Known Games:
// - [Akabei Soft2] [080529] G-senjou no Maou
// - [Type-Moon] [040326] Fate Stay Night
// - [Type-Moon] [051229] Fate Hollow Ataraxia
// - [Yurin Yurin] [130329] Sonohana 12 - Atelier no Koibito-tachi
// - [Yurin Yurin] [130531] Sonohana 13 - Tenshi no Akogare
// - [Yurin Yurin] [130830] Sonohana 14 - Tenshi-tachi no Harukoi
// - [Yurin Yurin] [131220] Sonohana 15 - Shirayuki no Kishi
// - [Yurin Yurin] [140328] Sonohana 16 - Tenshi-tachi no Yakusoku

#include "fmt/kirikiri/tlg_converter.h"
#include "err.h"
#include "fmt/kirikiri/tlg/tlg5_decoder.h"
#include "fmt/kirikiri/tlg/tlg6_decoder.h"

using namespace au;
using namespace au::fmt::kirikiri;
using namespace au::fmt::kirikiri::tlg;

static const bstr magic_tlg_0 = "TLG0.0\x00sds\x1A"_b;
static const bstr magic_tlg_5 = "TLG5.0\x00raw\x1A"_b;
static const bstr magic_tlg_6 = "TLG6.0\x00raw\x1A"_b;

static int guess_version(io::IO &io);
static std::unique_ptr<File> decode_proxy(int version, File &file);

static std::string extract_string(std::string &container)
{
    size_t size = 0;
    while (container.size() && container[0] >= '0' && container[0] <= '9')
    {
        size *= 10;
        size += container[0] - '0';
        container.erase(0, 1);
    }
    container.erase(0, 1);
    std::string str = container.substr(0, size);
    container.erase(0, size + 1);
    return str;
}

static std::unique_ptr<File> decode_tlg_0(File &file)
{
    size_t raw_data_size = file.io.read_u32_le();
    size_t raw_data_offset = file.io.tell();

    std::vector<std::pair<std::string, std::string>> tags;

    file.io.skip(raw_data_size);
    while (file.io.tell() < file.io.size())
    {
        std::string chunk_name = file.io.read(4).str();
        size_t chunk_size = file.io.read_u32_le();
        std::string chunk_data = file.io.read(chunk_size).str();

        if (chunk_name == "tags")
        {
            while (chunk_data != "")
            {
                std::string key = extract_string(chunk_data);
                std::string value = extract_string(chunk_data);
                tags.push_back(std::pair<std::string, std::string>(key, value));
            }
        }
        else
            throw err::NotSupportedError("Unknown chunk: " + chunk_name);
    }

    file.io.seek(raw_data_offset);
    int version = guess_version(file.io);
    if (version == -1)
        throw err::UnsupportedVersionError();
    return decode_proxy(version, file);
}

static std::unique_ptr<File> decode_tlg_5(File &file)
{
    Tlg5Decoder decoder;
    return decoder.decode(file);
}

static std::unique_ptr<File> decode_tlg_6(File &file)
{
    Tlg6Decoder decoder;
    return decoder.decode(file);
}

static int guess_version(io::IO &io)
{
    size_t pos = io.tell();
    if (io.read(magic_tlg_0.size()) == magic_tlg_0)
        return 0;

    io.seek(pos);
    if (io.read(magic_tlg_5.size()) == magic_tlg_5)
        return 5;

    io.seek(pos);
    if (io.read(magic_tlg_6.size()) == magic_tlg_6)
        return 6;

    return -1;
}

static std::unique_ptr<File> decode_proxy(int version, File &file)
{
    switch (version)
    {
        case 0:
            return decode_tlg_0(file);

        case 5:
            return decode_tlg_5(file);

        case 6:
            return decode_tlg_6(file);
    }
    throw std::logic_error("Unknown TLG version");
}

bool TlgConverter::is_recognized_internal(File &file) const
{
    return guess_version(file.io) >= 0;
}

std::unique_ptr<File> TlgConverter::decode_internal(File &file) const
{
    int version = guess_version(file.io);
    return decode_proxy(version, file);
}

static auto dummy = fmt::Registry::add<TlgConverter>("krkr/tlg");
