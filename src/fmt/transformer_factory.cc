#include <algorithm>
#include <functional>
#include "fmt/transformer_factory.h"

#include "fmt/alice_soft/afa_archive.h"
#include "fmt/alice_soft/aff_converter.h"
#include "fmt/alice_soft/ajp_converter.h"
#include "fmt/alice_soft/ald_archive.h"
#include "fmt/alice_soft/alk_archive.h"
#include "fmt/alice_soft/pm_converter.h"
#include "fmt/alice_soft/qnt_converter.h"
#include "fmt/bgi/arc_archive.h"
#include "fmt/bgi/cbg_converter.h"
#include "fmt/bgi/dsc_converter.h"
#include "fmt/bgi/sound_converter.h"
#include "fmt/cronus/pak_archive.h"
#include "fmt/cronus/pak_image_converter.h"
#include "fmt/french_bread/ex3_converter.h"
#include "fmt/french_bread/p_archive.h"
#include "fmt/fvp/bin_archive.h"
#include "fmt/fvp/nvsg_converter.h"
#include "fmt/glib/glib2_archive.h"
#include "fmt/glib/gml_archive.h"
#include "fmt/glib/pgx_converter.h"
#include "fmt/ivory/mbl_archive.h"
#include "fmt/ivory/prs_converter.h"
#include "fmt/ivory/wady_converter.h"
#include "fmt/key/g00_converter.h"
#include "fmt/key/nwa_converter.h"
#include "fmt/kid/cps_converter.h"
#include "fmt/kid/lnk_archive.h"
#include "fmt/kid/prt_converter.h"
#include "fmt/kid/waf_converter.h"
#include "fmt/kirikiri/tlg_converter.h"
#include "fmt/kirikiri/xp3_archive.h"
#include "fmt/liar_soft/lwg_archive.h"
#include "fmt/liar_soft/wcg_converter.h"
#include "fmt/liar_soft/xfl_archive.h"
#include "fmt/libido/arc_archive.h"
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
#include "fmt/riddle_soft/pac_archive.h"
#include "fmt/rpgmaker/rgss3a_archive.h"
#include "fmt/rpgmaker/rgssad_archive.h"
#include "fmt/rpgmaker/xyz_converter.h"
#include "fmt/tanuki_soft/tac_archive.h"
#include "fmt/touhou/anm_archive.h"
#include "fmt/touhou/pak1_archive.h"
#include "fmt/touhou/pak1_image_archive.h"
#include "fmt/touhou/pak1_sound_archive.h"
#include "fmt/touhou/pak2_archive.h"
#include "fmt/touhou/pak2_image_converter.h"
#include "fmt/touhou/pak2_sound_converter.h"
#include "fmt/touhou/pbg3_archive.h"
#include "fmt/touhou/pbg4_archive.h"
#include "fmt/touhou/pbgz_archive.h"
#include "fmt/touhou/tfbm_converter.h"
#include "fmt/touhou/tfcs_converter.h"
#include "fmt/touhou/tfpk_archive.h"
#include "fmt/touhou/tfwa_converter.h"
#include "fmt/touhou/tha1_archive.h"
#include "fmt/whale/dat_archive.h"
#include "fmt/yuka_script/ykc_archive.h"
#include "fmt/yuka_script/ykg_converter.h"

using namespace au;
using namespace au::fmt;

namespace
{
    using TransformerCreator = std::function<Transformer*()>;
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
    p->add("alice/afa",      []() { return new alice_soft::AfaArchive();     });
    p->add("alice/aff",      []() { return new alice_soft::AffConverter();   });
    p->add("alice/ajp",      []() { return new alice_soft::AjpConverter();   });
    p->add("alice/ald",      []() { return new alice_soft::AldArchive();     });
    p->add("alice/alk",      []() { return new alice_soft::AlkArchive();     });
    p->add("alice/pm",       []() { return new alice_soft::PmConverter();    });
    p->add("alice/qnt",      []() { return new alice_soft::QntConverter();   });
    p->add("bgi/arc",        []() { return new bgi::ArcArchive();            });
    p->add("bgi/cbg",        []() { return new bgi::CbgConverter();          });
    p->add("bgi/dsc",        []() { return new bgi::DscConverter();          });
    p->add("bgi/sound",      []() { return new bgi::SoundConverter();        });
    p->add("cronus/pak",     []() { return new cronus::PakArchive();         });
    p->add("cronus/pak-gfx", []() { return new cronus::PakImageConverter();  });
    p->add("fbread/ex3",     []() { return new french_bread::Ex3Converter(); });
    p->add("fbread/p",       []() { return new french_bread::PArchive();     });
    p->add("fvp/bin",        []() { return new fvp::BinArchive();            });
    p->add("fvp/nvsg",       []() { return new fvp::NvsgConverter();         });
    p->add("glib/g2",        []() { return new glib::Glib2Archive();         });
    p->add("glib/gml",       []() { return new glib::GmlArchive();           });
    p->add("glib/pgx",       []() { return new glib::PgxConverter();         });
    p->add("ivory/mbl",      []() { return new ivory::MblArchive();          });
    p->add("ivory/prs",      []() { return new ivory::PrsConverter();        });
    p->add("ivory/wady",     []() { return new ivory::WadyConverter();       });
    p->add("key/g00",        []() { return new key::G00Converter();          });
    p->add("key/nwa",        []() { return new key::NwaConverter();          });
    p->add("kid/cps",        []() { return new kid::CpsConverter();          });
    p->add("kid/lnk",        []() { return new kid::LnkArchive();            });
    p->add("kid/prt",        []() { return new kid::PrtConverter();          });
    p->add("kid/waf",        []() { return new kid::WafConverter();          });
    p->add("krkr/tlg",       []() { return new kirikiri::TlgConverter();     });
    p->add("krkr/xp3",       []() { return new kirikiri::Xp3Archive();       });
    p->add("liar/lwg",       []() { return new liar_soft::LwgArchive();      });
    p->add("liar/wcg",       []() { return new liar_soft::WcgConverter();    });
    p->add("liar/xfl",       []() { return new liar_soft::XflArchive();      });
    p->add("libido/arc",     []() { return new libido::ArcArchive();         });
    p->add("lizsoft/sotes",  []() { return new lizsoft::SotesConverter();    });
    p->add("minato/pac",     []() { return new minato_soft::PacArchive();    });
    p->add("ms/dds",         []() { return new microsoft::DdsConverter();    });
    p->add("ms/exe",         []() { return new microsoft::ExeArchive();      });
    p->add("nitro/npa",      []() { return new nitroplus::NpaArchive();      });
    p->add("nitro/npa_sg",   []() { return new nitroplus::NpaSgArchive();    });
    p->add("nitro/pak",      []() { return new nitroplus::PakArchive();      });
    p->add("nscripter/nsa",  []() { return new nscripter::NsaArchive();      });
    p->add("nscripter/sar",  []() { return new nscripter::SarArchive();      });
    p->add("nscripter/spb",  []() { return new nscripter::SpbConverter();    });
    p->add("nsystem/fjsys",  []() { return new nsystem::FjsysArchive();      });
    p->add("nsystem/mgd",    []() { return new nsystem::MgdConverter();      });
    p->add("propeller/mgr",  []() { return new propeller::MgrArchive();      });
    p->add("propeller/mpk",  []() { return new propeller::MpkArchive();      });
    p->add("qlie/dpng",      []() { return new qlie::DpngConverter();        });
    p->add("qlie/pack",      []() { return new qlie::PackArchive();          });
    p->add("renpy/rpa",      []() { return new renpy::RpaArchive();          });
    p->add("riddle/pac",     []() { return new riddle_soft::PacArchive();    });
    p->add("rm/rgss3a",      []() { return new rpgmaker::Rgss3aArchive();    });
    p->add("rm/rgssad",      []() { return new rpgmaker::RgssadArchive();    });
    p->add("rm/xyz",         []() { return new rpgmaker::XyzConverter();     });
    p->add("tanuki/tac",     []() { return new tanuki_soft::TacArchive();    });
    p->add("th/anm",         []() { return new touhou::AnmArchive();         });
    p->add("th/pak1",        []() { return new touhou::Pak1Archive();        });
    p->add("th/pak1-gfx",    []() { return new touhou::Pak1ImageArchive();   });
    p->add("th/pak1-sfx",    []() { return new touhou::Pak1SoundArchive();   });
    p->add("th/pak2",        []() { return new touhou::Pak2Archive();        });
    p->add("th/pak2-gfx",    []() { return new touhou::Pak2ImageConverter(); });
    p->add("th/pak2-sfx",    []() { return new touhou::Pak2SoundConverter(); });
    p->add("th/pbg3",        []() { return new touhou::Pbg3Archive();        });
    p->add("th/pbg4",        []() { return new touhou::Pbg4Archive();        });
    p->add("th/pbgz",        []() { return new touhou::PbgzArchive();        });
    p->add("th/tfbm",        []() { return new touhou::TfbmConverter();      });
    p->add("th/tfcs",        []() { return new touhou::TfcsConverter();      });
    p->add("th/tfpk",        []() { return new touhou::TfpkArchive();        });
    p->add("th/tfwa",        []() { return new touhou::TfwaConverter();      });
    p->add("th/tha1",        []() { return new touhou::Tha1Archive();        });
    p->add("whale/dat",      []() { return new whale::DatArchive();          });
    p->add("yuka/ykc",       []() { return new yuka_script::YkcArchive();    });
    p->add("yuka/ykg",       []() { return new yuka_script::YkgConverter();  });
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
