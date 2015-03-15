#include <functional>
#include "factory/archive_factory.h"
#include "formats/bgi/arc_archive.h"
#include "formats/french_bread/p_archive.h"
#include "formats/glib/glib2_archive.h"
#include "formats/glib/gml_archive.h"
#include "formats/ivory/mbl_archive.h"
#include "formats/kirikiri/xp3_archive.h"
#include "formats/liarsoft/lwg_archive.h"
#include "formats/liarsoft/xfl_archive.h"
#include "formats/microsoft/exe_archive.h"
#include "formats/minato_soft/pac_archive.h"
#include "formats/nitroplus/npa_archive.h"
#include "formats/nitroplus/npa_sg_archive.h"
#include "formats/nitroplus/pak_archive.h"
#include "formats/nscripter/nsa_archive.h"
#include "formats/nscripter/sar_archive.h"
#include "formats/nsystem/fjsys_archive.h"
#include "formats/qlie/pack_archive.h"
#include "formats/renpy/rpa_archive.h"
#include "formats/rpgmaker/rgssad_archive.h"
#include "formats/touhou/anm_archive.h"
#include "formats/touhou/pak1_archive.h"
#include "formats/touhou/pak2_archive.h"
#include "formats/touhou/pbg3_archive.h"
#include "formats/touhou/pbg4_archive.h"
#include "formats/touhou/pbgz_archive.h"
#include "formats/touhou/tha1_archive.h"
#include "formats/yukascript/ykc_archive.h"
using namespace Formats;

struct ArchiveFactory::Internals
{
    std::vector<std::pair<std::string, std::function<Archive*()>>> formats;

    void add(std::string format, std::function<Archive*()> creator)
    {
        formats.push_back(
            std::pair<std::string, std::function<Archive*()>>(format, creator));
    }
};

ArchiveFactory::ArchiveFactory() : internals(new Internals)
{
    internals->add("gml", []() { return new Glib::GmlArchive(); });
    internals->add("g2", []() { return new Glib::Glib2Archive(); });
    internals->add("th-pak1", []() { return new Touhou::Pak1Archive(); });
    internals->add("th-pak2", []() { return new Touhou::Pak2Archive(); });
    internals->add("tha1", []() { return new Touhou::Tha1Archive(); });
    internals->add("pbgz", []() { return new Touhou::PbgzArchive(); });
    internals->add("lwg", []() { return new LiarSoft::LwgArchive(); });
    internals->add("xfl", []() { return new LiarSoft::XflArchive(); });
    internals->add("pbg3", []() { return new Touhou::Pbg3Archive(); });
    internals->add("pbg4", []() { return new Touhou::Pbg4Archive(); });
    internals->add("pack", []() { return new QLiE::PackArchive(); });
    internals->add("pac", []() { return new MinatoSoft::PacArchive(); });
    internals->add("exe", []() { return new Microsoft::ExeArchive(); });
    internals->add("ykc", []() { return new YukaScript::YkcArchive(); });
    internals->add("rgssad", []() { return new RpgMaker::RgssadArchive(); });
    internals->add("fjsys", []() { return new NSystem::FjsysArchive(); });
    internals->add("arc", []() { return new Bgi::ArcArchive(); });
    internals->add("npa", []() { return new Nitroplus::NpaArchive(); });
    internals->add("xp3", []() { return new Kirikiri::Xp3Archive(); });
    internals->add("rpa", []() { return new Renpy::RpaArchive(); });
    internals->add("p", []() { return new FrenchBread::PArchive(); });
    internals->add("npa_sg", []() { return new Nitroplus::NpaSgArchive(); });
    internals->add("pak", []() { return new Nitroplus::PakArchive(); });
    internals->add("mbl", []() { return new Ivory::MblArchive(); });
    internals->add("nsa", []() { return new NScripter::NsaArchive(); });
    internals->add("sar", []() { return new NScripter::SarArchive(); });
    internals->add("anm", []() { return new Touhou::AnmArchive(); });
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
