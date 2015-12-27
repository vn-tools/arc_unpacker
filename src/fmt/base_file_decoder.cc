#include "fmt/base_file_decoder.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

NamingStrategy BaseFileDecoder::naming_strategy() const
{
    return NamingStrategy::FlatSibling;
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
