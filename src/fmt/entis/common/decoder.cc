#include "fmt/entis/common/decoder.h"

using namespace au;
using namespace au::fmt::entis::common;

Decoder::Decoder(const bstr &data) : bit_reader(data)
{
}

Decoder::~Decoder()
{
}
