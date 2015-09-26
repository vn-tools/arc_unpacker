#include "fmt/registry.h"
#include <algorithm>
#include <map>
#include "err.h"
#include "fmt/abstract_decoder.h"

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

const std::vector<std::string> Registry::get_names() const
{
    std::vector<std::string> names;
    for (auto &item : p->decoder_map)
        names.push_back(item.first);
    std::sort(names.begin(), names.end());
    return names;
}

std::unique_ptr<AbstractDecoder> Registry::create(const std::string &name) const
{
    for (auto &item : p->decoder_map)
        if (item.first == name)
            return item.second();
    throw err::UsageError("Invalid format: " + name);
}

void Registry::add(DecoderCreator creator, const std::string &name)
{
    if (p->decoder_map.find(name) != p->decoder_map.end())
    {
        throw std::logic_error(
            "Decoder with name " + name + " was already registered.");
    }
    p->decoder_map[name] = creator;
}

Registry &Registry::instance()
{
    static Registry instance;
    return instance;
}
