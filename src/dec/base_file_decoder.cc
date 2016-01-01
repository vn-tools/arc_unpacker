#include "dec/base_file_decoder.h"
#include "dec/idecoder_visitor.h"
#include "err.h"

using namespace au;
using namespace au::dec;

algo::NamingStrategy BaseFileDecoder::naming_strategy() const
{
    return algo::NamingStrategy::FlatSibling;
}

void BaseFileDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

std::unique_ptr<io::File> BaseFileDecoder::decode(
    const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
