#include "dec/kaguya/link4_archive_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK4"_b;

bool Link4ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

int Link4ArchiveDecoder::get_version() const
{
    return 4;
}

static auto _ = dec::register_decoder<Link4ArchiveDecoder>("kaguya/link4");
