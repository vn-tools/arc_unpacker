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

#include "arg_parser_decorator.h"

using namespace au;

ArgParserDecorator::ArgParserDecorator(
    const std::function<void(ArgParser &)> register_callback,
    const std::function<void(const ArgParser &)> parse_callback)
    : register_callback(register_callback), parse_callback(parse_callback)
{
}

void ArgParserDecorator::register_cli_options(ArgParser &arg_parser) const
{
    register_callback(arg_parser);
}

void ArgParserDecorator::parse_cli_options(const ArgParser &arg_parser) const
{
    parse_callback(arg_parser);
}
