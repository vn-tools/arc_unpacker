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

#include "dec/base_image_decoder.h"
#include "dec/idecoder_visitor.h"
#include "err.h"

using namespace au;
using namespace au::dec;

algo::NamingStrategy BaseImageDecoder::naming_strategy() const
{
    return algo::NamingStrategy::FlatSibling;
}

void BaseImageDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

res::Image BaseImageDecoder::decode(const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
