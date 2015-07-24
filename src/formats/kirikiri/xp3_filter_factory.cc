#include <functional>
#include <string>
#include <vector>
#include "formats/kirikiri/xp3_filter_factory.h"
#include "formats/kirikiri/xp3_filters/cxdec_comyu.h"
#include "formats/kirikiri/xp3_filters/cxdec_fha.h"
#include "formats/kirikiri/xp3_filters/cxdec_mahoyoru.h"
#include "formats/kirikiri/xp3_filters/fsn.h"
#include "formats/kirikiri/xp3_filters/noop.h"
using namespace Formats::Kirikiri;

struct Definition
{
    std::string name;
    std::string description;
    std::function<Xp3Filter*()> creator;
};

struct Xp3FilterFactory::Priv
{
    std::vector<struct Definition> definitions;

    void add(
        std::string name,
        std::function<Xp3Filter*()> creator,
        std::string description = "")
    {
        struct Definition d;
        d.name = name;
        d.description = description;
        d.creator = creator;
        definitions.push_back(d);
    }
};

Xp3FilterFactory::Xp3FilterFactory() : p(new Priv)
{
    p->add("comyu",    []() { return new Xp3Filters::CxdecComyu;    });
    p->add("fsn",      []() { return new Xp3Filters::Fsn;           });
    p->add("fha",      []() { return new Xp3Filters::CxdecFha;      });
    p->add("mahoyoru", []() { return new Xp3Filters::CxdecMahoYoru; });
    p->add("noop",     []() { return new Xp3Filters::Noop;          },
        "for unecrypted games");
}

Xp3FilterFactory::~Xp3FilterFactory()
{
}

void Xp3FilterFactory::add_cli_help(ArgParser &arg_parser)
{
    std::string help = "Selects XP3 decryption routine.\nPossible values:\n";
    for (auto &definition : p->definitions)
    {
        help += "- " + definition.name;
        if (definition.description != "")
            help += " (" + definition.description + ")";
        help += "\n";
    }
    arg_parser.add_help("--plugin=PLUGIN", help);
}

std::unique_ptr<Xp3Filter> Xp3FilterFactory::get_filter_from_cli_options(
    const ArgParser &arg_parser)
{
    const std::string plugin = arg_parser.get_switch("plugin");
    for (auto &definition : p->definitions)
        if (definition.name == plugin)
            return std::unique_ptr<Xp3Filter>(definition.creator());
    throw std::runtime_error("Unrecognized plugin: " + plugin);
}
