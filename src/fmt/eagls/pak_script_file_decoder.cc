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
    for (auto i : util::range(100))
    {
        auto name = file.io.read(32).str();
        file.io.skip(4);

        // after first zero, the "name" (whatever it is) should contain only
        // more zeros
        auto zero_index = name.find_first_of('\x00');
        if (zero_index == std::string::npos)
            return false;
        for (auto i : util::range(zero_index, name.size()))
            if (name[i] != '\x00')
                return false;
    }
    return true;
}

std::unique_ptr<File> PakScriptFileDecoder::decode_impl(File &file) const
{
    // According to Crass the offset, key and even the presence of LCG
    // vary for other games.

    file.io.seek(3600);
    auto data = file.io.read(file.io.size() - file.io.tell() - 1);
    s8 seed = file.io.read_u8();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(0, data.size(), 2))
        data[i] ^= key[lcg.next() % key.size()];

    auto output_file = std::make_unique<File>(file.name, data);
    output_file->change_extension("txt");
    return output_file;
}

static auto dummy = fmt::register_fmt<PakScriptFileDecoder>("eagls/pak-txt");
