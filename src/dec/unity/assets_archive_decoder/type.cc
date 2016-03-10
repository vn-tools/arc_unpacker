#include "dec/unity/assets_archive_decoder/type.h"

using namespace au;
using namespace au::dec::unity;

TypeV1::TypeV1(CustomStream &input_stream)
{
    type = input_stream.read_to_zero(256).str();
    name = input_stream.read_to_zero(256).str();
    size = input_stream.read<u32>();
    index = input_stream.read<u32>();
    is_array = input_stream.read<u32>() == 1;
    version = input_stream.read<u32>();
    meta_flag = input_stream.read<u32>();
}

TypeV2::TypeV2(CustomStream &input_stream)
{
    version = input_stream.read<u16>();
    tree_level = input_stream.read<u8>();
    is_array = input_stream.read<u8>() != 0;
    type_offset = input_stream.read<u32>();
    name_offset = input_stream.read<u32>();
    size = input_stream.read<u32>();
    index = input_stream.read<u32>();
    meta_flag = input_stream.read<u32>();
}
