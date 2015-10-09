#include "fmt/kirikiri/tlg_image_decoder.h"
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
static pix::Grid decode_proxy(int version, File &file);

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

static pix::Grid decode_tlg_0(File &file)
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

static pix::Grid decode_tlg_5(File &file)
{
    Tlg5Decoder decoder;
    return decoder.decode(file);
}

static pix::Grid decode_tlg_6(File &file)
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

static pix::Grid decode_proxy(int version, File &file)
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

bool TlgImageDecoder::is_recognized_impl(File &file) const
{
    return guess_version(file.io) >= 0;
}

pix::Grid TlgImageDecoder::decode_impl(File &file) const
{
    int version = guess_version(file.io);
    return decode_proxy(version, file);
}

static auto dummy = fmt::register_fmt<TlgImageDecoder>("krkr/tlg");
