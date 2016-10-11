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

#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace au {
namespace dec {

    class IDecoder;

    class Registry final
    {
    private:
        using DecoderCreator
            = std::function<std::shared_ptr<IDecoder>()>;

    public:
        ~Registry();
        static Registry &instance();
        static std::unique_ptr<Registry> create_mock();

        const std::vector<std::string> get_decoder_names() const;
        bool has_decoder(const std::string &name) const;
        void add_decoder(const std::string &name, DecoderCreator creator);
        std::shared_ptr<IDecoder> create_decoder(const std::string &name) const;

    private:
        Registry();

        struct Priv;
        std::unique_ptr<Priv> p;
    };

    template <typename T, typename ...Params> bool register_decoder(
        const std::string &name, Params&&... params)
    {
        Registry::instance().add_decoder(
            name, [=]() { return std::make_shared<T>(params...); });
        return true;
    }

} }
