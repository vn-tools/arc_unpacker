#include "fmt/audio_decoder.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

IDecoder::NamingStrategy AudioDecoder::naming_strategy() const
{
    return NamingStrategy::FlatSibling;
}

void AudioDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

res::Audio AudioDecoder::decode(const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
