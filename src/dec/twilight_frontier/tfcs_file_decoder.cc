#include "dec/twilight_frontier/tfcs_file_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::twilight_frontier;

static const bstr magic = "TFCS\x00"_b;

static void write_cell(io::BaseByteStream &output_stream, std::string cell)
{
    if (cell.find(",") != std::string::npos)
        cell = "\"" + algo::replace_all(cell, "\"", "\"\"") + "\"";
    output_stream.write(algo::sjis_to_utf8(cell));
}

bool TfcsFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> TfcsFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    io::MemoryStream uncompressed_stream(
        algo::pack::zlib_inflate(input_file.stream.read_to_eof()));

    auto output_file = std::make_unique<io::File>();
    output_file->path = input_file.path;
    output_file->path.change_extension("csv");

    const auto row_count = uncompressed_stream.read_le<u32>();
    for (const size_t i : algo::range(row_count))
    {
        const auto column_count = uncompressed_stream.read_le<u32>();
        for (const size_t j : algo::range(column_count))
        {
            const auto cell_size = uncompressed_stream.read_le<u32>();
            const auto cell = uncompressed_stream.read(cell_size).str();

            // escaping etc. is too boring
            write_cell(output_file->stream, cell);
            if (j != column_count - 1)
                output_file->stream.write(","_b);
        }
        output_file->stream.write("\n"_b);
    }

    return output_file;
}

static auto _ = dec::register_decoder<TfcsFileDecoder>(
    "twilight-frontier/tfcs");
