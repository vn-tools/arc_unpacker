#include "dec/eagls/pak_script_file_decoder.h"
#include "algo/crypt/lcg.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;

bool PakScriptFileDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat")
        || input_file.stream.size() < 3600)
    {
        return false;
    }

    for (auto i : algo::range(100))
    {
        auto name = input_file.stream.read(32).str();
        input_file.stream.skip(4);

        // after first zero, the "name" (whatever it is) should contain only
        // more zeros
        auto zero_index = name.find_first_of('\x00');
        if (zero_index == std::string::npos)
            return false;
        for (auto i : algo::range(zero_index, name.size()))
            if (name[i] != '\x00')
                return false;
    }

    return true;
}

std::unique_ptr<io::File> PakScriptFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    // According to Crass the offset, key and even the presence of LCG
    // vary for other games.

    const auto offset = 3600;
    input_file.stream.seek(offset);
    auto data = input_file.stream.read(input_file.stream.size() - offset - 1);
    s8 seed = input_file.stream.read_u8();
    algo::crypt::Lcg lcg(algo::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : algo::range(0, data.size(), 2))
        data[i] ^= key[lcg.next() % key.size()];

    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("txt");
    return output_file;
}

static auto _ = dec::register_decoder<PakScriptFileDecoder>("eagls/pak-script");
