#include "dec/entis/common/base_decoder.h"
#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::dec::entis::common;

void BaseDecoder::set_input(const bstr &data)
{
    bit_reader.reset(new io::MsbBitReader(data));
}
