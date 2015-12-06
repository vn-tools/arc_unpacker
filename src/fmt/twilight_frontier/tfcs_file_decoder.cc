#include "fmt/twilight_frontier/tfcs_file_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"
#include "util/algo/str.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const bstr magic = "TFCS\x00"_b;

static void write_cell(io::Stream &output_stream, std::string cell)
{
    if (cell.find(",") != std::string::npos)
        cell = "\"" + util::algo::replace_all(cell, "\"", "\"\"") + "\"";
    output_stream.write(util::sjis_to_utf8(cell));
}

bool TfcsFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> TfcsFileDecoder::decode_impl(
    io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    size_t compressed_size = input_file.stream.read_u32_le();
    size_t original_size = input_file.stream.read_u32_le();
    io::MemoryStream uncompressed_stream(
        util::pack::zlib_inflate(input_file.stream.read_to_eof()));

    auto output_file = std::make_unique<io::File>();
    output_file->path = input_file.path;
    output_file->path.change_extension("csv");

    size_t row_count = uncompressed_stream.read_u32_le();
    for (size_t i : util::range(row_count))
    {
        size_t column_count = uncompressed_stream.read_u32_le();
        for (size_t j : util::range(column_count))
        {
            size_t cell_size = uncompressed_stream.read_u32_le();
            std::string cell = uncompressed_stream.read(cell_size).str();

            // escaping etc. is too boring
            write_cell(output_file->stream, cell);
            if (j != column_count - 1)
                output_file->stream.write(","_b);
        }
        output_file->stream.write("\n"_b);
    }

    return output_file;
}

static auto dummy
    = fmt::register_fmt<TfcsFileDecoder>("twilight-frontier/tfcs");
