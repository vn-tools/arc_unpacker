#include "dec/entis/common/base_decoder.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::entis::common;

void BaseDecoder::set_input(const bstr &data)
{
    bit_stream.reset(new io::MsbBitStream(data));
}
