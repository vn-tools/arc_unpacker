// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/registry.h"
#include <algorithm>
#include <map>
#include "dec/idecoder.h"
#include "err.h"

using namespace au::dec;

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

std::shared_ptr<IDecoder>
    Registry::create_decoder(const std::string &name) const
{
    if (!has_decoder(name))
        throw err::UsageError("Unknown decoder: " + name);
    return p->decoder_map[name]();
}

void Registry::add_decoder(const std::string &name, DecoderCreator creator)
{
    if (has_decoder(name))
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

std::unique_ptr<Registry> Registry::create_mock()
{
    return std::unique_ptr<Registry>(new Registry());
}
