#include "fmt/entis/common/abstract_decoder.h"

using namespace au;
using namespace au::fmt::entis::common;

AbstractDecoder::AbstractDecoder()
{
}

AbstractDecoder::~AbstractDecoder()
{
}

void AbstractDecoder::set_input(const bstr &data)
{
    bit_reader.reset(new io::MsbBitReader(data));
}
