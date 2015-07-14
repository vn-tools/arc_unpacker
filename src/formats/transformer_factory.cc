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
#include "formats/propeller/mgr_archive.h"
#include "formats/propeller/mpk_archive.h"
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

struct TransformerFactory::Priv
{
    std::vector<std::pair<std::string, TransformerCreator>> formats;

    void add(std::string format, TransformerCreator creator)
    {
        formats.push_back(
            std::pair<std::string, TransformerCreator>(format, creator));
    }
};

TransformerFactory::TransformerFactory() : p(new Priv)
{
    p->add("mgr", []() { return new Propeller::MgrArchive(); });
    p->add("mpk", []() { return new Propeller::MpkArchive(); });
    p->add("fvp", []() { return new Fvp::BinArchive(); });
    p->add("gml", []() { return new Glib::GmlArchive(); });
    p->add("g2", []() { return new Glib::Glib2Archive(); });
    p->add("th-pak1", []() { return new Touhou::Pak1Archive(); });
    p->add("th-pak2", []() { return new Touhou::Pak2Archive(); });
    #ifdef HAVE_OPENSSL_RSA_H
    p->add("tfpk", []() { return new Touhou::TfpkArchive(); });
    #endif
    p->add("tha1", []() { return new Touhou::Tha1Archive(); });
    p->add("pbgz", []() { return new Touhou::PbgzArchive(); });
    p->add("lwg", []() { return new LiarSoft::LwgArchive(); });
    p->add("xfl", []() { return new LiarSoft::XflArchive(); });
    p->add("pbg3", []() { return new Touhou::Pbg3Archive(); });
    p->add("pbg4", []() { return new Touhou::Pbg4Archive(); });
    p->add("pack", []() { return new QLiE::PackArchive(); });
    p->add("pac", []() { return new MinatoSoft::PacArchive(); });
    p->add("exe", []() { return new Microsoft::ExeArchive(); });
    p->add("ykc", []() { return new YukaScript::YkcArchive(); });
    p->add("rgssad", []() { return new RpgMaker::RgssadArchive(); });
    p->add("fjsys", []() { return new NSystem::FjsysArchive(); });
    p->add("arc", []() { return new Bgi::ArcArchive(); });
    p->add("npa", []() { return new Nitroplus::NpaArchive(); });
    p->add("xp3", []() { return new Kirikiri::Xp3Archive(); });
    p->add("rpa", []() { return new Renpy::RpaArchive(); });
    p->add("p", []() { return new FrenchBread::PArchive(); });
    p->add("npa_sg", []() { return new Nitroplus::NpaSgArchive(); });
    p->add("pak", []() { return new Nitroplus::PakArchive(); });
    p->add("mbl", []() { return new Ivory::MblArchive(); });
    p->add("nsa", []() { return new NScripter::NsaArchive(); });
    p->add("sar", []() { return new NScripter::SarArchive(); });
    p->add("anm", []() { return new Touhou::AnmArchive(); });
    p->add("dat-whale", []() { return new Whale::DatArchive(); });

    p->add("nvsg", []() { return new Fvp::NvsgConverter(); });
    p->add("dds", []() { return new Microsoft::DdsConverter(); });
    p->add("pgx", []() { return new Glib::PgxConverter(); });
    p->add("wcg", []() { return new LiarSoft::WcgConverter(); });
    p->add("dpng", []() { return new QLiE::DpngConverter(); });
    p->add("bgi-sound", []() { return new Bgi::SoundConverter(); });
    p->add("ex3", []() { return new FrenchBread::Ex3Converter(); });
    p->add("tlg", []() { return new Kirikiri::TlgConverter(); });
    p->add("ykg", []() { return new YukaScript::YkgConverter(); });
    p->add("cbg", []() { return new Bgi::CbgConverter(); });
    p->add("xyz", []() { return new RpgMaker::XyzConverter(); });
    p->add("mgd", []() { return new NSystem::MgdConverter(); });
    p->add("g00", []() { return new Key::G00Converter(); });
    p->add("sotes", []() { return new Lizsoft::SotesConverter(); });
    p->add("nwa", []() { return new Key::NwaConverter(); });
    p->add("prs", []() { return new Ivory::PrsConverter(); });
    p->add("spb", []() { return new NScripter::SpbConverter(); });
}

TransformerFactory::~TransformerFactory()
{
}

const std::vector<std::string> TransformerFactory::get_formats() const
{
    std::vector<std::string> formats;
    for (auto &item : p->formats)
        formats.push_back(item.first);

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
    for (auto &item : p->formats)
    {
        if (item.first == format)
            return std::unique_ptr<Transformer>(item.second());
    }
    throw std::runtime_error("Invalid format: " + format);
}
