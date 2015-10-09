#include "fmt/eagls/pak_script_file_decoder.h"
#include "util/crypt/lcg.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;

bool PakScriptFileDecoder::is_recognized_impl(File &file) const
{
    if (!file.has_extension("dat") || file.io.size() < 3600)
        return false;
    // header should consist mostly of zeros
    auto data = file.io.read(3600);
    size_t zeros = 0;
    for (auto i : util::range(data.size()))
        zeros += !data[i];
    return zeros > data.size() * 0.8;
}

std::unique_ptr<File> PakScriptFileDecoder::decode_impl(File &file) const
{
    // According to Crass the offset, key and even the presence of LCG
    // vary for other games.

    file.io.skip(3600);
    auto data = file.io.read(file.io.size() - file.io.tell() - 1);
    s8 seed = file.io.read_u8();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(0, data.size(), 2))
        data[i] ^= key[lcg.next() % key.size()];

    auto output_file = std::make_unique<File>();
    output_file->name = file.name;
    output_file->change_extension("txt");
    output_file->io.write(data);
    return output_file;
}

static auto dummy = fmt::register_fmt<PakScriptFileDecoder>("eagls/pak-txt");
