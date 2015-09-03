#include <functional>
#include <string>
#include <vector>
#include "fmt/kirikiri/xp3_filter_factory.h"
#include "fmt/kirikiri/xp3_filters/cxdec_comyu.h"
#include "fmt/kirikiri/xp3_filters/cxdec_fha.h"
#include "fmt/kirikiri/xp3_filters/cxdec_mahoyoru.h"
#include "fmt/kirikiri/xp3_filters/fsn.h"
#include "fmt/kirikiri/xp3_filters/noop.h"

using namespace au::fmt::kirikiri;

namespace
{
    struct Definition
    {
        std::string name;
        std::string description;
        std::function<Xp3Filter*()> creator;
    };
}

struct Xp3FilterFactory::Priv
{
    std::vector<struct Definition> definitions;

    void add(
        const std::string &name,
        const std::string &description,
        std::function<Xp3Filter*()> creator)
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
    p->add(
        "noop",
        "Unecrypted games",
        []() { return new xp3_filters::Noop; });

    p->add(
        "comyu",
        "Comyu - Kuroi Ryuu to Yasashii Oukoku",
        []() { return new xp3_filters::CxdecComyu; });

    p->add(
        "fsn",
        "Fate/Stay Night",
        []() { return new xp3_filters::Fsn; });

    p->add(
        "fha",
        "Fate/Hollow Ataraxia",
        []() { return new xp3_filters::CxdecFha; });

    p->add(
        "mahoyoru",
        "Mahou Tsukai no Yoru",
        []() { return new xp3_filters::CxdecMahoYoru; });
}

Xp3FilterFactory::~Xp3FilterFactory()
{
}

void Xp3FilterFactory::register_cli_options(ArgParser &arg_parser)
{
    auto sw = arg_parser.register_switch({"-p", "--plugin"})
        ->set_value_name("PLUGIN")
        ->set_description("Selects XP3 decryption routine.");
    for (auto &definition : p->definitions)
        sw->add_possible_value(definition.name, definition.description);
}

std::unique_ptr<Xp3Filter> Xp3FilterFactory::get_filter_from_cli_options(
    const ArgParser &arg_parser)
{
    if (!arg_parser.has_switch("plugin"))
        throw std::runtime_error("Plugin not specified");
    const std::string plugin = arg_parser.get_switch("plugin");
    for (auto &definition : p->definitions)
        if (definition.name == plugin)
            return std::unique_ptr<Xp3Filter>(definition.creator());
    throw std::runtime_error("Unrecognized plugin: " + plugin);
}
