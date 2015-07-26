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
#include "formats/propeller/mgr_archive.h"
#include "formats/propeller/mpk_archive.h"
#include "formats/qlie/dpng_converter.h"
#include "formats/qlie/pack_archive.h"
#include "formats/renpy/rpa_archive.h"
#include "formats/rpgmaker/rgssad_archive.h"
#include "formats/rpgmaker/xyz_converter.h"
#ifdef HAVE_OPENSSL_RSA_H
#include "formats/tanuki_soft/tac_archive.h"
#endif
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

using namespace au;
using namespace au::fmt;

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
    #ifdef HAVE_OPENSSL_RSA_H
    p->add("tac",       []() { return new tanuki_soft::TacArchive(); });
    p->add("tfpk",      []() { return new touhou::TfpkArchive();     });
    #endif
    p->add("mgr",       []() { return new propeller::MgrArchive();   });
    p->add("mpk",       []() { return new propeller::MpkArchive();   });
    p->add("fvp",       []() { return new fvp::BinArchive();         });
    p->add("gml",       []() { return new glib::GmlArchive();        });
    p->add("g2",        []() { return new glib::Glib2Archive();      });
    p->add("th-pak1",   []() { return new touhou::Pak1Archive();     });
    p->add("th-pak2",   []() { return new touhou::Pak2Archive();     });
    p->add("tha1",      []() { return new touhou::Tha1Archive();     });
    p->add("pbgz",      []() { return new touhou::PbgzArchive();     });
    p->add("lwg",       []() { return new liarsoft::LwgArchive();    });
    p->add("xfl",       []() { return new liarsoft::XflArchive();    });
    p->add("pbg3",      []() { return new touhou::Pbg3Archive();     });
    p->add("pbg4",      []() { return new touhou::Pbg4Archive();     });
    p->add("pack",      []() { return new qlie::PackArchive();       });
    p->add("pac",       []() { return new minato_soft::PacArchive(); });
    p->add("exe",       []() { return new microsoft::ExeArchive();   });
    p->add("ykc",       []() { return new yukascript::YkcArchive();  });
    p->add("rgssad",    []() { return new rpgmaker::RgssadArchive(); });
    p->add("fjsys",     []() { return new nsystem::FjsysArchive();   });
    p->add("arc",       []() { return new bgi::ArcArchive();         });
    p->add("npa",       []() { return new nitroplus::NpaArchive();   });
    p->add("xp3",       []() { return new kirikiri::Xp3Archive();    });
    p->add("rpa",       []() { return new renpy::RpaArchive();       });
    p->add("p",         []() { return new french_bread::PArchive();  });
    p->add("npa_sg",    []() { return new nitroplus::NpaSgArchive(); });
    p->add("pak",       []() { return new nitroplus::PakArchive();   });
    p->add("mbl",       []() { return new ivory::MblArchive();       });
    p->add("nsa",       []() { return new nscripter::NsaArchive();   });
    p->add("sar",       []() { return new nscripter::SarArchive();   });
    p->add("anm",       []() { return new touhou::AnmArchive();      });
    p->add("dat-whale", []() { return new whale::DatArchive();       });

    p->add("nvsg",      []() { return new fvp::NvsgConverter();         });
    p->add("dds",       []() { return new microsoft::DdsConverter();    });
    p->add("pgx",       []() { return new glib::PgxConverter();         });
    p->add("wcg",       []() { return new liarsoft::WcgConverter();     });
    p->add("dpng",      []() { return new qlie::DpngConverter();        });
    p->add("bgi-sound", []() { return new bgi::SoundConverter();        });
    p->add("ex3",       []() { return new french_bread::Ex3Converter(); });
    p->add("tlg",       []() { return new kirikiri::TlgConverter();     });
    p->add("ykg",       []() { return new yukascript::YkgConverter();   });
    p->add("cbg",       []() { return new bgi::CbgConverter();          });
    p->add("xyz",       []() { return new rpgmaker::XyzConverter();     });
    p->add("mgd",       []() { return new nsystem::MgdConverter();      });
    p->add("g00",       []() { return new key::G00Converter();          });
    p->add("sotes",     []() { return new lizsoft::SotesConverter();    });
    p->add("nwa",       []() { return new key::NwaConverter();          });
    p->add("prs",       []() { return new ivory::PrsConverter();        });
    p->add("spb",       []() { return new nscripter::SpbConverter();    });
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
