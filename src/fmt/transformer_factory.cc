#include <algorithm>
#include <functional>
#include "fmt/transformer_factory.h"

#include "fmt/bgi/arc_archive.h"
#include "fmt/bgi/cbg_converter.h"
#include "fmt/bgi/sound_converter.h"
#include "fmt/french_bread/ex3_converter.h"
#include "fmt/french_bread/p_archive.h"
#include "fmt/fvp/bin_archive.h"
#include "fmt/fvp/nvsg_converter.h"
#include "fmt/glib/glib2_archive.h"
#include "fmt/glib/gml_archive.h"
#include "fmt/glib/pgx_converter.h"
#include "fmt/ivory/mbl_archive.h"
#include "fmt/ivory/prs_converter.h"
#include "fmt/key/g00_converter.h"
#include "fmt/key/nwa_converter.h"
#include "fmt/kirikiri/tlg_converter.h"
#include "fmt/kirikiri/xp3_archive.h"
#include "fmt/liar_soft/lwg_archive.h"
#include "fmt/liar_soft/wcg_converter.h"
#include "fmt/liar_soft/xfl_archive.h"
#include "fmt/lizsoft/sotes_converter.h"
#include "fmt/microsoft/dds_converter.h"
#include "fmt/microsoft/exe_archive.h"
#include "fmt/minato_soft/pac_archive.h"
#include "fmt/nitroplus/npa_archive.h"
#include "fmt/nitroplus/npa_sg_archive.h"
#include "fmt/nitroplus/pak_archive.h"
#include "fmt/nscripter/nsa_archive.h"
#include "fmt/nscripter/sar_archive.h"
#include "fmt/nscripter/spb_converter.h"
#include "fmt/nsystem/fjsys_archive.h"
#include "fmt/nsystem/mgd_converter.h"
#include "fmt/propeller/mgr_archive.h"
#include "fmt/propeller/mpk_archive.h"
#include "fmt/qlie/dpng_converter.h"
#include "fmt/qlie/pack_archive.h"
#include "fmt/renpy/rpa_archive.h"
#include "fmt/rpgmaker/rgssad_archive.h"
#include "fmt/rpgmaker/xyz_converter.h"
#ifdef HAVE_OPENSSL_RSA_H
#include "fmt/tanuki_soft/tac_archive.h"
#endif
#include "fmt/touhou/anm_archive.h"
#include "fmt/touhou/pak1_archive.h"
#include "fmt/touhou/pak2_archive.h"
#include "fmt/touhou/pbg3_archive.h"
#include "fmt/touhou/pbg4_archive.h"
#include "fmt/touhou/pbgz_archive.h"
#include "fmt/whale/dat_archive.h"
#ifdef HAVE_OPENSSL_RSA_H
#include "fmt/touhou/tfpk_archive.h"
#endif
#include "fmt/touhou/tha1_archive.h"
#include "fmt/yuka_script/ykc_archive.h"
#include "fmt/yuka_script/ykg_converter.h"

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
    p->add("lwg",       []() { return new liar_soft::LwgArchive();   });
    p->add("xfl",       []() { return new liar_soft::XflArchive();   });
    p->add("pbg3",      []() { return new touhou::Pbg3Archive();     });
    p->add("pbg4",      []() { return new touhou::Pbg4Archive();     });
    p->add("pack",      []() { return new qlie::PackArchive();       });
    p->add("pac",       []() { return new minato_soft::PacArchive(); });
    p->add("exe",       []() { return new microsoft::ExeArchive();   });
    p->add("ykc",       []() { return new yuka_script::YkcArchive(); });
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
    p->add("wcg",       []() { return new liar_soft::WcgConverter();    });
    p->add("dpng",      []() { return new qlie::DpngConverter();        });
    p->add("bgi-sound", []() { return new bgi::SoundConverter();        });
    p->add("ex3",       []() { return new french_bread::Ex3Converter(); });
    p->add("tlg",       []() { return new kirikiri::TlgConverter();     });
    p->add("ykg",       []() { return new yuka_script::YkgConverter();  });
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
