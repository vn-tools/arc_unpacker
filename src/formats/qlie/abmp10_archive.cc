#include "formats/qlie/abmp10_archive.h"
#include "util/encoding.h"
using namespace Formats::QLiE;

static const std::string magic10("abmp10\0\0\0\0\0\0\0\0\0\0", 16);
static const std::string magic11("abmp11\0\0\0\0\0\0\0\0\0\0", 16);
static const std::string magic12("abmp12\0\0\0\0\0\0\0\0\0\0", 16);
static const std::string magic_imgdat10("abimgdat10\0\0\0\0\0\0", 16);
static const std::string magic_imgdat11("abimgdat11\0\0\0\0\0\0", 16);
static const std::string magic_imgdat13("abimgdat13\0\0\0\0\0\0", 16);
static const std::string magic_imgdat14("abimgdat14\0\0\0\0\0\0", 16);
static const std::string magic_snddat10("absnddat10\0\0\0\0\0\0", 16);
static const std::string magic_snddat11("absnddat11\0\0\0\0\0\0", 16);
static const std::string magic_data10("abdata10\0\0\0\0\0\0\0\0", 16);
static const std::string magic_data11("abdata11\0\0\0\0\0\0\0\0", 16);
static const std::string magic_data12("abdata12\0\0\0\0\0\0\0\0", 16);
static const std::string magic_data13("abdata13\0\0\0\0\0\0\0\0", 16);
static const std::string magic_image10("abimage10\0\0\0\0\0\0\0", 16);
static const std::string magic_sound10("absound10\0\0\0\0\0\0\0", 16);

static int guess_version(IO &arc_io)
{
    std::string magic = arc_io.read(16);
    if (magic == magic10)
        return 10;
    if (magic == magic11)
        return 11;
    if (magic == magic12)
        return 12;
    return -1;
}

static std::unique_ptr<File> read_file(IO &arc_io)
{
    std::string magic = arc_io.read(16);
    std::string encoded_name = arc_io.read(arc_io.read_u16_le());
    std::string name = sjis_to_utf8(encoded_name);

    if (magic == magic_snddat11
        || magic == magic_imgdat11
        || magic == magic_imgdat13
        || magic == magic_imgdat14)
    {
        arc_io.skip(arc_io.read_u16_le());
    }
    else if (magic != magic_imgdat10 && magic != magic_snddat10)
        throw std::runtime_error("Unknown image magic " + magic);

    arc_io.skip(1);

    if (magic == magic_imgdat14)
        arc_io.skip(76);
    else if (magic == magic_imgdat13)
        arc_io.skip(12);

    size_t len = arc_io.read_u32_le();
    if (len == 0)
        return nullptr;

    std::unique_ptr<File> subfile(new File);
    subfile->io.write_from_io(arc_io, len);
    subfile->name = (name == "" ? "unknown" : name) + ".dat";
    subfile->guess_extension();
    return subfile;
}

bool Abmp10Archive::is_recognized_internal(File &arc_file) const
{
    return guess_version(arc_file.io) >= 0;
}

void Abmp10Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    int version = guess_version(arc_file.io);

    while (arc_file.io.tell() < arc_file.io.size())
    {
        std::string magic = arc_file.io.read(16);
        if (magic == magic_data10
        || magic == magic_data11
        || magic == magic_data12
        || magic == magic_data13)
        {
            //interesting
            size_t size = arc_file.io.read_u32_le();
            arc_file.io.skip(size);
        }
        else if (magic == magic_image10)
        {
            size_t image_count = arc_file.io.read_u8();
            for (size_t i = 0; i < image_count; i++)
            {
                auto subfile = read_file(arc_file.io);
                if (subfile != nullptr)
                    file_saver.save(std::move(subfile));
            }
        }
        else if (magic == magic_sound10)
        {
            size_t sound_count = arc_file.io.read_u8();
            for (size_t i = 0; i < sound_count; i++)
            {
                auto subfile = read_file(arc_file.io);
                if (subfile != nullptr)
                    file_saver.save(std::move(subfile));
            }
        }
        else
            throw std::runtime_error("Unknown section " + magic);
    }
}
