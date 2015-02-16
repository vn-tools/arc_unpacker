#include "formats/arc/pack_archive/abmp10.h"
#include "string_ex.h"

namespace
{
    const std::string magic10("abmp10\0\0\0\0\0\0\0\0\0\0", 16);
    const std::string magic11("abmp11\0\0\0\0\0\0\0\0\0\0", 16);
    const std::string magic12("abmp12\0\0\0\0\0\0\0\0\0\0", 16);
    const std::string magic_imgdat10("abimgdat10\0\0\0\0\0\0", 16);
    const std::string magic_imgdat11("abimgdat11\0\0\0\0\0\0", 16);
    const std::string magic_imgdat13("abimgdat13\0\0\0\0\0\0", 16);
    const std::string magic_imgdat14("abimgdat14\0\0\0\0\0\0", 16);
    const std::string magic_snddat10("absnddat10\0\0\0\0\0\0", 16);
    const std::string magic_snddat11("absnddat11\0\0\0\0\0\0", 16);
    const std::string magic_data10("abdata10\0\0\0\0\0\0\0\0", 16);
    const std::string magic_data11("abdata11\0\0\0\0\0\0\0\0", 16);
    const std::string magic_data12("abdata12\0\0\0\0\0\0\0\0", 16);
    const std::string magic_data13("abdata13\0\0\0\0\0\0\0\0", 16);
    const std::string magic_image10("abimage10\0\0\0\0\0\0\0", 16);
    const std::string magic_sound10("absound10\0\0\0\0\0\0\0", 16);

    int get_version(IO &b_io)
    {
        if (b_io.size() < 16)
            return 0;

        std::string magic = b_io.read(16);
        if (magic == magic10)
            return 10;
        if (magic == magic11)
            return 11;
        if (magic == magic12)
            return 12;
        return 0;
    }

    std::unique_ptr<File> read_subfile(
        IO &b_io, const std::string base_name)
    {
        std::string magic = b_io.read(16);
        std::string encoded_name = b_io.read(b_io.read_u16_le());
        std::string name = convert_encoding(encoded_name, "cp932", "utf-8");

        if (magic == magic_snddat11
        || magic == magic_imgdat11
        || magic == magic_imgdat13
        || magic == magic_imgdat14)
        {
            b_io.skip(b_io.read_u16_le());
        }
        else if (magic != magic_imgdat10 && magic != magic_snddat10)
            throw std::runtime_error("Unknown image magic " + magic);

        b_io.skip(1);

        if (magic == magic_imgdat14)
            b_io.skip(76);
        else if (magic == magic_imgdat13)
            b_io.skip(12);

        size_t len = b_io.read_u32_le();
        if (len == 0)
            return nullptr;

        std::unique_ptr<File> subfile(new File);
        subfile->io.write_from_io(b_io, len);
        subfile->name = base_name + "_" + name + ".dat";
        subfile->guess_extension();
        return subfile;
    }
}

bool abmp10_unpack(std::vector<std::unique_ptr<File>> &files, File &b_file)
{
    b_file.io.seek(0);
    int version = get_version(b_file.io);
    if (!version)
        return false;

    while (b_file.io.tell() < b_file.io.size())
    {
        std::string magic = b_file.io.read(16);
        if (magic == magic_data10
        || magic == magic_data11
        || magic == magic_data12
        || magic == magic_data13)
        {
            //interesting
            size_t size = b_file.io.read_u32_le();
            b_file.io.skip(size);
        }
        else if (magic == magic_image10)
        {
            size_t image_count = b_file.io.read_u8();
            for (size_t i = 0; i < image_count; i ++)
            {
                auto subfile = read_subfile(
                    b_file.io, b_file.name + "_" + stoi(i));
                files.push_back(std::move(subfile));
            }
        }
        else if (magic == magic_sound10)
        {
            size_t sound_count = b_file.io.read_u8();
            for (size_t i = 0; i < sound_count; i ++)
            {
                auto subfile = read_subfile(
                    b_file.io, b_file.name + "_" + stoi(i));
                files.push_back(std::move(subfile));
            }
        }
        else
            throw std::runtime_error("Unknown section " + magic);
    }
    return true;
}
