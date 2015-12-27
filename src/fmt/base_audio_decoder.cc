#include "fmt/base_audio_decoder.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

NamingStrategy BaseAudioDecoder::naming_strategy() const
{
    return NamingStrategy::FlatSibling;
}

void BaseAudioDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

res::Audio BaseAudioDecoder::decode(const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
