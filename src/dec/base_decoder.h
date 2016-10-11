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

#include <vector>
#include "dec/idecoder.h"
#include "dec/registry.h" // for child decoders

namespace au {
namespace dec {

    class BaseDecoder
        : public IDecoder, public std::enable_shared_from_this<IDecoder>
    {
    public:
        virtual ~BaseDecoder() {}

        std::vector<ArgParserDecorator>
            get_arg_parser_decorators() const override;

        virtual bool is_recognized(io::File &input_file) const override;

        virtual std::vector<std::string> get_linked_formats() const override;

    protected:
        void add_arg_parser_decorator(const ArgParserDecorator &decorator);

        void add_arg_parser_decorator(
            const std::function<void(ArgParser &)> register_callback,
            const std::function<void(const ArgParser &)> parse_callback);

        virtual bool is_recognized_impl(io::File &input_file) const = 0;

    private:
        std::vector<ArgParserDecorator> arg_parser_decorators;
    };

} }
