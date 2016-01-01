#include "dec/entis/common/abstract_decoder.h"
#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::dec::entis::common;

void AbstractDecoder::set_input(const bstr &data)
{
    bit_reader.reset(new io::MsbBitReader(data));
}
