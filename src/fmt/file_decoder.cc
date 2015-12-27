#include "fmt/file_decoder.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

IDecoder::NamingStrategy FileDecoder::naming_strategy() const
{
    return NamingStrategy::FlatSibling;
}

void FileDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

std::unique_ptr<io::File> FileDecoder::decode(
    const Logger &logger, io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(logger, file);
}
