#include "dec/unity/assets_archive_decoder/object_id_table.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::unity;

ObjectIdTable::ObjectIdTable(CustomStream &input_stream)
{
    const auto entry_count = input_stream.read<u32>();
    for (const auto i : algo::range(entry_count))
        push_back(std::make_unique<ObjectId>(input_stream));
    input_stream.align(4);
}
