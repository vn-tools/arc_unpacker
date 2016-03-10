#include "dec/unity/assets_archive_decoder/file_id_table.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::unity;

FileIdTable::FileIdTable(
    CustomStream &input_stream, const FileIdReader file_id_reader)
{
    const auto entry_count = input_stream.read<u32>();
    for (const auto i : algo::range(entry_count))
    {
        push_back(file_id_reader(input_stream));
    }
}
