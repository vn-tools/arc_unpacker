#include "dec/unity/assets_archive_decoder/util.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::unity;

Hash::Hash()
{
}

Hash::Hash(CustomStream &input_stream)
{
    for (const auto i : algo::range(size()))
        operator[](i) = input_stream.read<u8>();
}
