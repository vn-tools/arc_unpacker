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
