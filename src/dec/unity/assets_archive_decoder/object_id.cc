#include "dec/unity/assets_archive_decoder/object_id.h"

using namespace au;
using namespace au::dec::unity;

ObjectId::ObjectId(CustomStream &input_stream)
{
    serialized_file_index = input_stream.read<u32>();
    identifier_in_file = input_stream.read<u64>();
}
