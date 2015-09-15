// FJSYS archive
//
// Company:   -
// Engine:    NSystem
// Extension: none
//
// Known games:
// - [Fuguriya] [060929] Sonohana 1
// - [Fuguriya] [070529] Sonohana 2 - Watashi no Ouji-sama
// - [Fuguriya] [070714] Sonohana 3 - Anata to Koibito Tsunagi
// - [Fuguriya] [080606] Sonohana 4 - Itoshisa no Photograph
// - [Fuguriya] [080725] Sonohana 5 - Anata o Suki na Shiawase
// - [Fuguriya] [081205] Sonohana 6 - Kuchibiru to Kiss de Tsubuyaite
// - [Fuguriya] [091211] Sonohana 7 - Amakute Hoshikute Torokeru Chuu
// - [Fuguriya] [100212] Sonohana 8 - Tenshi no Hanabira Zome
// - [Fuguriya] [101129] Sonohana 9 - Amakute Otona no Torokeru Chuu
// - [Fuguriya] [111120] Sonohana 10 - Lily Platinum
// - [Yurin Yurin] [121115] Sonohana 11 - Michael no Otome-tachi
// - [Yurin Yurin] [100925] Hanahira!

#include "fmt/nsystem/fjsys_archive.h"
#include "fmt/nsystem/mgd_converter.h"
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

struct FjsysArchive::Priv final
{
    MgdConverter mgd_converter;
};

FjsysArchive::FjsysArchive() : p(new Priv)
{
    add_transformer(&p->mgd_converter);
}

FjsysArchive::~FjsysArchive()
{
}

bool FjsysArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void FjsysArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    std::unique_ptr<Header> header = read_header(arc_file.io);
    for (auto i : util::range(header->file_count))
        file_saver.save(read_file(arc_file.io, *header));
}

static auto dummy = fmt::Registry::add<FjsysArchive>("nsystem/fjsys");
