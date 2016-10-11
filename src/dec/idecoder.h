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

#include <memory>
#include <vector>
#include "algo/naming_strategies.h"
#include "arg_parser_decorator.h"
#include "io/file.h"

namespace au {
namespace dec {

    class IDecoderVisitor;

    class IDecoder
    {
    public:
        virtual ~IDecoder() {}

        virtual void accept(IDecoderVisitor &visitor) const = 0;

        virtual std::vector<ArgParserDecorator>
            get_arg_parser_decorators() const = 0;

        virtual bool is_recognized(io::File &input_file) const = 0;

        virtual std::vector<std::string> get_linked_formats() const = 0;

        virtual algo::NamingStrategy naming_strategy() const = 0;
    };

} }
