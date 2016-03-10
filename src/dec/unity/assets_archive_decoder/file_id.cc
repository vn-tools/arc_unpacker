#include "dec/unity/assets_archive_decoder/file_id.h"

using namespace au;
using namespace au::dec::unity;

FileIdV1::FileIdV1(CustomStream &input_stream)
{
    guid = Hash(input_stream),
    type = input_stream.read<u32>();
    path = input_stream.read_to_zero().str();
}

FileIdV2::FileIdV2(CustomStream &input_stream)
{
    asset_path = input_stream.read_to_zero().str();
    guid = Hash(input_stream),
    type = input_stream.read<u32>();
    path = input_stream.read_to_zero().str();
}
