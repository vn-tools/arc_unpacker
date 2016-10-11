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

#include "dec/base_decoder.h"

using namespace au;
using namespace au::dec;

std::vector<ArgParserDecorator> BaseDecoder::get_arg_parser_decorators() const
{
    return arg_parser_decorators;
}

std::vector<std::string> BaseDecoder::get_linked_formats() const
{
    return {};
}

void BaseDecoder::add_arg_parser_decorator(const ArgParserDecorator &decorator)
{
    arg_parser_decorators.push_back(decorator);
}

void BaseDecoder::add_arg_parser_decorator(
    const std::function<void(ArgParser &)> register_callback,
    const std::function<void(const ArgParser &)> parse_callback)
{
    const ArgParserDecorator decorator(register_callback, parse_callback);
    add_arg_parser_decorator(decorator);
}

bool BaseDecoder::is_recognized(io::File &input_file) const
{
    try
    {
        input_file.stream.seek(0);
        return is_recognized_impl(input_file);
    }
    catch (...)
    {
        return false;
    }
}
