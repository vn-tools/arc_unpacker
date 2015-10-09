#include "fmt/registry.h"
#include <algorithm>
#include <map>
#include "err.h"
#include "fmt/idecoder.h"

using namespace au::fmt;

struct Registry::Priv final
{
    std::map<std::string, DecoderCreator> decoder_map;
};

Registry::Registry() : p(new Priv)
{
}

Registry::~Registry()
{
}

const std::vector<std::string> Registry::get_decoder_names() const
{
    std::vector<std::string> names;
    for (auto &item : p->decoder_map)
        names.push_back(item.first);
    std::sort(names.begin(), names.end());
    return names;
}

bool Registry::has_decoder(const std::string &name) const
{
    return p->decoder_map.find(name) != p->decoder_map.end();
}

std::unique_ptr<IDecoder>
    Registry::create_decoder(const std::string &name) const
{
    if (!has_decoder(name))
        throw err::UsageError("Unknown decoder: " + name);
    return p->decoder_map[name]();
}

void Registry::add_decoder(const std::string &name, DecoderCreator creator)
{
    if (has_decoder(name))
        throw std::logic_error(
            "Decoder with name " + name + " was already registered.");
    p->decoder_map[name] = creator;
}

Registry &Registry::instance()
{
    static Registry instance;
    return instance;
}

std::unique_ptr<Registry> Registry::create_mock()
{
    return std::unique_ptr<Registry>(new Registry());
}
