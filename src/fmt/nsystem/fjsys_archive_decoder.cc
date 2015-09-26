#include "fmt/nsystem/fjsys_archive_decoder.h"
#include "fmt/nsystem/mgd_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nsystem;

namespace
{
    struct Header final
    {
        size_t header_size;
        size_t file_names_size;
        size_t file_count;
    };
}

static const bstr magic = "FJSYS\x00\x00\x00"_b;

static std::unique_ptr<Header> read_header(io::IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    header->header_size = arc_io.read_u32_le();
    header->file_names_size = arc_io.read_u32_le();
    header->file_count = arc_io.read_u32_le();
    arc_io.skip(64);
    return header;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const Header &header)
{
    std::unique_ptr<File> file(new File);
    size_t file_name_offset = arc_io.read_u32_le();
    size_t data_size = arc_io.read_u32_le();
    size_t data_offset = static_cast<size_t>(arc_io.read_u64_le());
    size_t old_pos = arc_io.tell();
    size_t file_names_start = header.header_size - header.file_names_size;

    arc_io.seek(file_name_offset + file_names_start);
    file->name = arc_io.read_to_zero().str();

    arc_io.seek(data_offset);
    file->io.write_from_io(arc_io, data_size);

    arc_io.seek(old_pos);
    return file;
}

struct FjsysArchiveDecoder::Priv final
{
    MgdImageDecoder mgd_image_decoder;
};

FjsysArchiveDecoder::FjsysArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->mgd_image_decoder);
}

FjsysArchiveDecoder::~FjsysArchiveDecoder()
{
}

bool FjsysArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void FjsysArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());

    std::unique_ptr<Header> header = read_header(arc_file.io);
    for (auto i : util::range(header->file_count))
        saver.save(read_file(arc_file.io, *header));
}

static auto dummy = fmt::Registry::add<FjsysArchiveDecoder>("nsystem/fjsys");
