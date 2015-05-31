#include <algorithm>
#include <functional>
#include "formats/transformer_factory.h"

#include "formats/bgi/arc_archive.h"
#include "formats/bgi/cbg_converter.h"
#include "formats/bgi/sound_converter.h"
#include "formats/french_bread/ex3_converter.h"
#include "formats/french_bread/p_archive.h"
#include "formats/fvp/bin_archive.h"
#include "formats/fvp/nvsg_converter.h"
#include "formats/glib/glib2_archive.h"
#include "formats/glib/gml_archive.h"
#include "formats/glib/pgx_converter.h"
#include "formats/ivory/mbl_archive.h"
#include "formats/ivory/prs_converter.h"
#include "formats/key/g00_converter.h"
#include "formats/key/nwa_converter.h"
#include "formats/kirikiri/tlg_converter.h"
#include "formats/kirikiri/xp3_archive.h"
#include "formats/liarsoft/lwg_archive.h"
#include "formats/liarsoft/wcg_converter.h"
#include "formats/liarsoft/xfl_archive.h"
#include "formats/lizsoft/sotes_converter.h"
#include "formats/microsoft/dds_converter.h"
#include "formats/microsoft/exe_archive.h"
#include "formats/minato_soft/pac_archive.h"
#include "formats/nitroplus/npa_archive.h"
#include "formats/nitroplus/npa_sg_archive.h"
#include "formats/nitroplus/pak_archive.h"
#include "formats/nscripter/nsa_archive.h"
#include "formats/nscripter/sar_archive.h"
#include "formats/nscripter/spb_converter.h"
#include "formats/nsystem/fjsys_archive.h"
#include "formats/nsystem/mgd_converter.h"
#include "formats/qlie/dpng_converter.h"
#include "formats/qlie/pack_archive.h"
#include "formats/renpy/rpa_archive.h"
#include "formats/rpgmaker/rgssad_archive.h"
#include "formats/rpgmaker/xyz_converter.h"
#include "formats/touhou/anm_archive.h"
#include "formats/touhou/pak1_archive.h"
#include "formats/touhou/pak2_archive.h"
#include "formats/touhou/pbg3_archive.h"
#include "formats/touhou/pbg4_archive.h"
#include "formats/touhou/pbgz_archive.h"
#include "formats/whale/dat_archive.h"
#ifdef HAVE_OPENSSL_RSA_H
#include "formats/touhou/tfpk_archive.h"
#endif
#include "formats/touhou/tha1_archive.h"
#include "formats/yukascript/ykc_archive.h"
#include "formats/yukascript/ykg_converter.h"

using namespace Formats;

namespace
{
    typedef std::function<Transformer*()> TransformerCreator;
}

struct TransformerFactory::Internals
{
    std::vector<std::pair<std::string, TransformerCreator>> formats;

    void add(std::string format, TransformerCreator creator)
    {
        formats.push_back(
            std::pair<std::string, TransformerCreator>(format, creator));
    }
};

TransformerFactory::TransformerFactory() : internals(new Internals)
{
    internals->add("fvp", []() { return new Fvp::BinArchive(); });
    internals->add("gml", []() { return new Glib::GmlArchive(); });
    internals->add("g2", []() { return new Glib::Glib2Archive(); });
    internals->add("th-pak1", []() { return new Touhou::Pak1Archive(); });
    internals->add("th-pak2", []() { return new Touhou::Pak2Archive(); });
    #ifdef HAVE_OPENSSL_RSA_H
    internals->add("tfpk", []() { return new Touhou::TfpkArchive(); });
    #endif
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
    internals->add("dat-whale", []() { return new Whale::DatArchive(); });

    internals->add("nvsg", []() { return new Fvp::NvsgConverter(); });
    internals->add("dds", []() { return new Microsoft::DdsConverter(); });
    internals->add("pgx", []() { return new Glib::PgxConverter(); });
    internals->add("wcg", []() { return new LiarSoft::WcgConverter(); });
    internals->add("dpng", []() { return new QLiE::DpngConverter(); });
    internals->add("bgi-sound", []() { return new Bgi::SoundConverter(); });
    internals->add("ex3", []() { return new FrenchBread::Ex3Converter(); });
    internals->add("tlg", []() { return new Kirikiri::TlgConverter(); });
    internals->add("ykg", []() { return new YukaScript::YkgConverter(); });
    internals->add("cbg", []() { return new Bgi::CbgConverter(); });
    internals->add("xyz", []() { return new RpgMaker::XyzConverter(); });
    internals->add("mgd", []() { return new NSystem::MgdConverter(); });
    internals->add("g00", []() { return new Key::G00Converter(); });
    internals->add("sotes", []() { return new Lizsoft::SotesConverter(); });
    internals->add("nwa", []() { return new Key::NwaConverter(); });
    internals->add("prs", []() { return new Ivory::PrsConverter(); });
    internals->add("spb", []() { return new NScripter::SpbConverter(); });
}

TransformerFactory::~TransformerFactory()
{
}

const std::vector<std::string> TransformerFactory::get_formats() const
{
    std::vector<std::string> formats;
    for (auto &p : internals->formats)
        formats.push_back(p.first);

    std::sort(
        formats.begin(),
        formats.end(),
        [](const std::string &left, const std::string &right)
        {
            return left < right;
        });

    return formats;
}

std::unique_ptr<Transformer> TransformerFactory::create(
    const std::string &format) const
{
    for (auto &p : internals->formats)
    {
        if (p.first == format)
            return std::unique_ptr<Transformer>(p.second());
    }
    throw std::runtime_error("Invalid format: " + format);
}
