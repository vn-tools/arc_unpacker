// EAGLS PAK script file
//
// Company:   TechArts
// Engine:    Enhanced Adventure Game Language System
// Extension: .dat
// Archives:  PAK
//
// Known games:
// - [SQUEEZ] [050922] Honoo no Haramase Tenkousei

#include "fmt/eagls/pak_script_converter.h"
#include "util/crypt/lcg.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;

bool PakScriptConverter::is_recognized_internal(File &file) const
{
    if (!file.has_extension("dat") || file.io.size() < 3600)
        return false;
    //header should consist mostly of zeros
    auto data = file.io.read(3600);
    size_t zeros = 0;
    for (auto i : util::range(data.size()))
        zeros += !data[i];
    return zeros > data.size() * 0.8;
}

std::unique_ptr<File> PakScriptConverter::decode_internal(File &file) const
{
    // According to Crass the offset, key and even the presence of LCG
    // vary for other games.

    file.io.skip(3600);
    auto data = file.io.read(file.io.size() - file.io.tell() - 1);
    s8 seed = file.io.read_u8();
    util::crypt::Lcg lcg(util::crypt::LcgKind::MicrosoftVisualC, seed);
    for (auto i : util::range(0, data.size(), 2))
        data[i] ^= key[lcg.next() % key.size()];

    std::unique_ptr<File> output_file(new File);
    output_file->name = file.name;
    output_file->change_extension("txt");
    output_file->io.write(data);
    return output_file;
}

static auto dummy = fmt::Registry::add<PakScriptConverter>("eagls/pak-txt");
