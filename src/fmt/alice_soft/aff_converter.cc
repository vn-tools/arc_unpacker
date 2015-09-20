#include "fmt/alice_soft/aff_converter.h"
#include <algorithm>
#include "util/range.h"

// Doesn't encode anything, just wraps real files.

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "AFF\x00"_b;
static const bstr key =
    "\xC8\xBB\x8F\xB7\xED\x43\x99\x4A\xA2\x7E\x5B\xB0\x68\x18\xF8\x88"_b;

bool AffConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> AffConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    file.io.skip(4);
    auto size = file.io.read_u32_le();
    file.io.skip(4);

    std::unique_ptr<File> output_file(new File);
    for (auto i : util::range(64))
        if (!file.io.eof())
            output_file->io.write_u8(file.io.read_u8() ^ key[i % key.size()]);
    output_file->io.write_from_io(file.io);
    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<AffConverter>("alice/aff");
