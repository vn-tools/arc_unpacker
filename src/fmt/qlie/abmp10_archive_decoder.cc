#include "fmt/qlie/abmp10_archive_decoder.h"
#include "err.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::qlie;

static const bstr magic10 = "abmp10\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic11 = "abmp11\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic12 = "abmp12\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic_imgdat10 = "abimgdat10\0\0\0\0\0\0"_b;
static const bstr magic_imgdat11 = "abimgdat11\0\0\0\0\0\0"_b;
static const bstr magic_imgdat13 = "abimgdat13\0\0\0\0\0\0"_b;
static const bstr magic_imgdat14 = "abimgdat14\0\0\0\0\0\0"_b;
static const bstr magic_snddat10 = "absnddat10\0\0\0\0\0\0"_b;
static const bstr magic_snddat11 = "absnddat11\0\0\0\0\0\0"_b;
static const bstr magic_data10 = "abdata10\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data11 = "abdata11\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data12 = "abdata12\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data13 = "abdata13\0\0\0\0\0\0\0\0"_b;
static const bstr magic_image10 = "abimage10\0\0\0\0\0\0\0"_b;
static const bstr magic_sound10 = "absound10\0\0\0\0\0\0\0"_b;

static int guess_version(io::IO &arc_io)
{
    bstr magic = arc_io.read(16);
    if (magic == magic10)
        return 10;
    if (magic == magic11)
        return 11;
    if (magic == magic12)
        return 12;
    return -1;
}

static std::unique_ptr<File> read_file(io::IO &arc_io)
{
    bstr magic = arc_io.read(16);
    bstr encoded_name = arc_io.read(arc_io.read_u16_le());
    std::string name = util::sjis_to_utf8(encoded_name).str();

    if (magic == magic_snddat11
        || magic == magic_imgdat11
        || magic == magic_imgdat13
        || magic == magic_imgdat14)
    {
        arc_io.skip(arc_io.read_u16_le());
    }
    else if (magic != magic_imgdat10 && magic != magic_snddat10)
    {
        throw err::NotSupportedError(util::format(
            "Unknown image magic: %s", magic.get<char>()));
    }

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
    return subfile;
}

bool Abmp10ArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return guess_version(arc_file.io) >= 0;
}

void Abmp10ArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &saver) const
{
    int version = guess_version(arc_file.io);

    while (arc_file.io.tell() < arc_file.io.size())
    {
        bstr magic = arc_file.io.read(16);
        if (magic == magic_data10
            || magic == magic_data11
            || magic == magic_data12
            || magic == magic_data13)
        {
            // interesting
            size_t size = arc_file.io.read_u32_le();
            arc_file.io.skip(size);
        }
        else if (magic == magic_image10 || magic == magic_sound10)
        {
            size_t file_count = arc_file.io.read_u8();
            for (auto i : util::range(file_count))
            {
                auto subfile = read_file(arc_file.io);
                if (subfile)
                {
                    subfile->guess_extension();
                    saver.save(std::move(subfile));
                }
            }
        }
        else
        {
            throw err::NotSupportedError(util::format(
                "Unknown section: %s", magic.get<char>()));
        }
    }
}

static auto dummy = fmt::Registry::add<Abmp10ArchiveDecoder>("qlie/abmp10");
