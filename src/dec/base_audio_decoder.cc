#include "dec/base_audio_decoder.h"
#include "dec/idecoder_visitor.h"
#include "err.h"

using namespace au;
using namespace au::dec;

algo::NamingStrategy BaseAudioDecoder::naming_strategy() const
{
    return algo::NamingStrategy::FlatSibling;
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
