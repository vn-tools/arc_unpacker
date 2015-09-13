#include "fmt/entis/common/decoder.h"

using namespace au;
using namespace au::fmt::entis::common;

Decoder::Decoder()
{
}

Decoder::~Decoder()
{
}

void Decoder::set_input(const bstr &data)
{
    bit_reader.reset(new io::BitReader(data));
}
