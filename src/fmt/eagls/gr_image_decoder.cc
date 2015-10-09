#include "fmt/eagls/gr_image_decoder.h"
#include <algorithm>
#include "io/buffered_io.h"
#include "util/crypt/lcg.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;
static const u32 xor_value = 0x75BD924;

static size_t guess_output_size(const bstr &data)
{
    io::BufferedIO tmp_io(util::pack::lzss_decompress_bytewise(data, 30));
    tmp_io.skip(10);
    auto pixels_start = tmp_io.read_u32_le();
    tmp_io.skip(4);
    auto width = tmp_io.read_u32_le();
    auto height = tmp_io.read_u32_le();
    tmp_io.skip(2);
    auto bpp = tmp_io.read_u16_le();
    auto stride = ((width + 3) / 4) * 4;
    return pixels_start + stride * height * (bpp >> 3);
}

bool GrImageDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("gr");
}

std::unique_ptr<File> GrImageDecoder::decode_impl(File &file) const
{
    // According to Crass the offset, key and LCG kind vary for other games.

    auto data = file.io.read(file.io.size() - 1);
    auto seed = file.io.read_u8() ^ xor_value;

    util::crypt::Lcg lcg(util::crypt::LcgKind::ParkMillerRevised, seed);
    for (auto i : util::range(0, std::min<size_t>(0x174B, data.size())))
        data[i] ^= key[lcg.next() % key.size()];

    auto output_size = guess_output_size(data);
    data = util::pack::lzss_decompress_bytewise(data, output_size);

    auto output_file = std::make_unique<File>();
    output_file->name = file.name;
    output_file->change_extension("bmp");
    output_file->io.write(data);
    return output_file;
}

static auto dummy = fmt::register_fmt<GrImageDecoder>("eagls/gr");
