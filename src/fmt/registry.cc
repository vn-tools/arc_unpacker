#include <map>
#include "err.h"
#include "fmt/registry.h"

using namespace au::fmt;

struct Registry::Priv final
{
    std::map<std::string, TransformerCreator> transformer_map;
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
    for (auto &item : p->transformer_map)
        names.push_back(item.first);
    std::sort(names.begin(), names.end());
    return names;
}

std::unique_ptr<Transformer> Registry::create(const std::string &name) const
{
    for (auto &item : p->transformer_map)
        if (item.first == name)
            return item.second();
    throw err::UsageError("Invalid format: " + name);
}

void Registry::add(TransformerCreator creator, const std::string &name)
{
    if (p->transformer_map.find(name) != p->transformer_map.end())
    {
        throw std::logic_error(
            "Transformer with name " + name + " was already registered.");
    }
    p->transformer_map[name] = creator;
}

Registry &Registry::instance()
{
    static Registry instance;
    return instance;
}
