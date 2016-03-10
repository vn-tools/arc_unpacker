#include "dec/unity/assets_archive_decoder/object_info_table.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::unity;

ObjectInfoTableV1::ObjectInfoTableV1(
    CustomStream &input_stream, const ObjectInfoReader object_info_reader)
{
    const auto entry_count = input_stream.read<u32>();
    for (const auto i : algo::range(entry_count))
    {
        const auto path_id = input_stream.read<u32>();
        operator[](path_id) = object_info_reader(input_stream);
    }
}

ObjectInfoTableV2::ObjectInfoTableV2(
    CustomStream &input_stream, const ObjectInfoReader object_info_reader)
{
    const auto entry_count = input_stream.read<u32>();
    for (const auto i : algo::range(entry_count))
    {
        input_stream.align(4);
        const auto path_id = input_stream.read<u64>();
        operator[](path_id) = object_info_reader(input_stream);
    }
}
