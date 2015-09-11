#include "err.h"
#include "fmt/entis/common/gamma_decoder.h"

using namespace au;
using namespace au::fmt::entis;
using namespace au::fmt::entis::common;

int common::get_gamma_code(io::BitReader &bit_reader)
{
    if (bit_reader.eof())
        return 0;
    if (!bit_reader.get(1))
        return 1;
    auto base = 2;
    auto code = 0;
    while (true)
    {
        if (bit_reader.eof())
            return 0;
        code = (code << 1) | bit_reader.get(1);
        if (bit_reader.eof())
            return 0;
        if (!bit_reader.get(1))
            return code + base;
        base <<= 1;
    }
}
