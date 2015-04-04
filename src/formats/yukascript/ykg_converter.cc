// YKG image
//
// Company:   -
// Engine:a   YukaScript
// Extension: .ykg
// Archives:  YKC

#include <memory>
#include "formats/yukascript/ykg_converter.h"
using namespace Formats::YukaScript;

namespace
{
    const std::string magic("YKG000", 6);

    typedef struct
    {
        bool encrypted;
        size_t data_offset;
        size_t data_size;
        size_t regions_offset;
        size_t regions_size;
    } Header;

    typedef struct
    {
        size_t width;
        size_t height;
        size_t x;
        size_t y;
    } Region;

    std::unique_ptr<Header> read_header(IO &file_io)
    {
        std::unique_ptr<Header> header(new Header);
        header->encrypted = file_io.read_u16_le() > 0;

        size_t header_size = file_io.read_u32_le();
        if (header_size != 64)
            throw std::runtime_error("Unexpected header size");
        file_io.skip(28);

        header->data_offset = file_io.read_u32_le();
        header->data_size = file_io.read_u32_le();
        file_io.skip(8);

        header->regions_offset = file_io.read_u32_le();
        header->regions_size = file_io.read_u32_le();
        return header;
    }

    std::vector<std::unique_ptr<Region>> read_regions(
        IO &file_io, Header &header)
    {
        std::vector<std::unique_ptr<Region>> regions;

        file_io.seek(header.regions_offset);
        size_t region_count = header.regions_size / 64;
        for (size_t i = 0; i < region_count; i ++)
        {
            std::unique_ptr<Region> region(new Region);
            region->x = file_io.read_u32_le();
            region->y = file_io.read_u32_le();
            region->width = file_io.read_u32_le();
            region->height = file_io.read_u32_le();
            file_io.skip(48);
            regions.push_back(std::move(region));
        }
        return regions;
    }

    std::unique_ptr<File> decode_png(File &file, Header &header)
    {
        file.io.seek(header.data_offset);
        std::string data = file.io.read(header.data_size);
        if (data.substr(1, 3) != "GNP")
        {
            throw std::runtime_error(
                "Decoding non-PNG based YKG images is not supported");
        }
        data[1] = 'P';
        data[2] = 'N';
        data[3] = 'G';

        std::unique_ptr<File> output_file(new File);
        output_file->io.write(data);
        output_file->name = file.name;
        output_file->change_extension(".png");
        return output_file;
    }
}

std::unique_ptr<File> YkgConverter::decode_internal(File &file) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a YKG image");

    std::unique_ptr<Header> header = read_header(file.io);
    if (header->encrypted)
    {
        throw std::runtime_error(
            "Decoding encrypted YKG images is not supported");
    }

    read_regions(file.io, *header);
    return decode_png(file, *header);
}
