#include "fmt/base_image_decoder.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

NamingStrategy BaseImageDecoder::naming_strategy() const
{
    return NamingStrategy::FlatSibling;
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
