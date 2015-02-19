#include <functional>
#include "factory/archive_factory.h"
#include "formats/arc/arc_archive.h"
#include "formats/arc/exe_archive.h"
#include "formats/arc/fjsys_archive.h"
#include "formats/arc/mbl_archive.h"
#include "formats/arc/npa_archive.h"
#include "formats/arc/npa_sg_archive.h"
#include "formats/arc/nsa_archive.h"
#include "formats/arc/p_archive.h"
#include "formats/arc/pac_archive.h"
#include "formats/arc/pack_archive.h"
#include "formats/arc/pak_archive.h"
#include "formats/arc/pbg3_archive.h"
#include "formats/arc/pbg4_archive.h"
#include "formats/arc/rgssad_archive.h"
#include "formats/arc/rpa_archive.h"
#include "formats/arc/sar_archive.h"
#include "formats/arc/xp3_archive.h"
#include "formats/arc/ykc_archive.h"

struct ArchiveFactory::Internals
{
    std::vector<std::pair<std::string, std::function<Archive*()>>> formats;

    void add_format(std::string format, std::function<Archive*()> creator)
    {
        formats.push_back(
            std::pair<std::string, std::function<Archive*()>>(format, creator));
    }
};

ArchiveFactory::ArchiveFactory() : internals(new Internals)
{
    internals->add_format("pbg3", []() { return new Pbg3Archive(); });
    internals->add_format("pbg4", []() { return new Pbg4Archive(); });
    internals->add_format("pack", []() { return new PackArchive(); });
    internals->add_format("pac", []() { return new PacArchive(); });
    internals->add_format("exe", []() { return new ExeArchive(); });
    internals->add_format("ykc", []() { return new YkcArchive(); });
    internals->add_format("rgssad", []() { return new RgssadArchive(); });
    internals->add_format("fjsys", []() { return new FjsysArchive(); });
    internals->add_format("arc", []() { return new ArcArchive(); });
    internals->add_format("npa", []() { return new NpaArchive(); });
    internals->add_format("xp3", []() { return new Xp3Archive(); });
    internals->add_format("rpa", []() { return new RpaArchive(); });
    internals->add_format("p", []() { return new PArchive(); });
    internals->add_format("npa_sg", []() { return new NpaSgArchive(); });
    internals->add_format("pak", []() { return new PakArchive(); });
    internals->add_format("mbl", []() { return new MblArchive(); });
    internals->add_format("nsa", []() { return new NsaArchive(); });
    internals->add_format("sar", []() { return new SarArchive(); });
}

ArchiveFactory::~ArchiveFactory()
{
}

const std::vector<std::string> ArchiveFactory::get_formats() const
{
    std::vector<std::string> formats;
    for (auto &p : internals->formats)
        formats.push_back(p.first);
    return formats;
}

Archive *ArchiveFactory::create_archive(const std::string format) const
{
    for (auto &p : internals->formats)
        if (p.first == format)
            return p.second();
    throw std::runtime_error("Invalid archive format: " + format);
}
