#include "fmt/kirikiri/xp3_filter_registry.h"
#include "fmt/kirikiri/cxdec.h"
#include "util/plugin_mgr.hh"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kirikiri;

static std::function<void(Xp3Filter&)> cxdec(
    u16 key1,
    u16 key2,
    const std::array<size_t, 3> order1,
    const std::array<size_t, 8> order2,
    const std::array<size_t, 6> order3)
{
    return [=](Xp3Filter &filter)
    {
        filter.decoder = create_cxdec_filter(
            filter.arc_path, key1, key2, order1, order2, order3);
    };
}

struct Xp3FilterRegistry::Priv final
{
    util::PluginManager<std::function<void(Xp3Filter&)>> plugin_mgr;
    std::function<void(Xp3Filter&)> filter_decorator;
};

void Xp3FilterRegistry::register_cli_options(ArgParser &arg_parser) const
{
    p->plugin_mgr.register_cli_options(
        arg_parser, "Selects XP3 decryption routine.");
}

void Xp3FilterRegistry::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter_decorator = p->plugin_mgr.get_from_cli_options(arg_parser, true);
}

void Xp3FilterRegistry::set_decoder(Xp3Filter &filter)
{
    if (p->filter_decorator)
        p->filter_decorator(filter);
}

Xp3FilterRegistry::Xp3FilterRegistry() : p(new Priv)
{
    p->plugin_mgr.add(
        "noop", "Unecrypted games",
        [](Xp3Filter &filter) { filter.decoder = [](bstr &, u32) { }; });

    p->plugin_mgr.add(
        "fsn", "Fate/Stay Night",
        [](Xp3Filter &filter)
        {
            filter.decoder = [](bstr &data, u32)
            {
                for (auto i : util::range(data.size()))
                    data[i] ^= 0x36;
                if (data.size() > 0x2EA29)
                    data[0x2EA29] ^= 3;
                if (data.size() > 0x13)
                    data[0x13] ^= 1;
            };
        });

    p->plugin_mgr.add(
        "comyu", "Comyu - Kuroi Ryuu to Yasashii Oukoku",
        cxdec(0x1A3, 0x0B6, {0,1,2}, {0,7,5,6,3,1,4,2}, {4,3,2,1,5,0}));

    p->plugin_mgr.add(
        "fha", "Fate/Hollow Ataraxia",
        cxdec(0x143, 0x787, {0,1,2}, {0,1,2,3,4,5,6,7}, {0,1,2,3,4,5}));

    p->plugin_mgr.add(
        "mahoyoru", "Mahou Tsukai no Yoru",
        cxdec(0x22A, 0x2A2, {1,0,2}, {7,6,5,1,0,3,4,2}, {3,2,1,4,5,0}));

    p->plugin_mgr.add(
        "mixed-xor", "Gokkun! Onii-chan Milk ~Punipuni Oppai na Imouto to~",
        [](Xp3Filter &filter)
        {
            filter.decoder = [](bstr &data, u32 key)
            {
                for (auto i : util::range(0, data.size(), 2))
                    data[i] ^= key;
                for (auto i : util::range(1, data.size(), 2))
                    data[i] ^= i;
            };
        });
}

Xp3FilterRegistry::~Xp3FilterRegistry()
{
}
