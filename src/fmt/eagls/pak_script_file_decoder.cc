#include "fmt/eagls/pak_script_file_decoder.h"
#include "util/crypt/lcg.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;

bool PakScriptFileDecoder::is_recognized_impl(File &input_file) const
{
    if (!input_file.has_extension("dat") || input_file.stream.size() < 3600)
        return false;
    for (auto i : util::range(100))
    {
        auto name = input_file.stream.read(32).str();
        input_file.stream.skip(4);

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

std::unique_ptr<File> PakScriptFileDecoder::decode_impl(File &input_file) const
{
    // According to Crass the offset, key and even the presence of LCG
    // vary for other games.

    const auto offset = 3600;
    input_file.stream.seek(offset);
    auto data = input_file.stream.read(input_file.stream.size() - offset - 1);
    s8 seed = input_file.stream.read_u8();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(0, data.size(), 2))
        data[i] ^= key[lcg.next() % key.size()];

    auto output_file = std::make_unique<File>(input_file.name, data);
    output_file->change_extension("txt");
    return output_file;
}

static auto dummy = fmt::register_fmt<PakScriptFileDecoder>("eagls/pak-script");
