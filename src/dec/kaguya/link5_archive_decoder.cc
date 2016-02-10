#include "dec/kaguya/link5_archive_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK5"_b;

bool Link5ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

int Link5ArchiveDecoder::get_version() const
{
    return 5;
}

static auto _ = dec::register_decoder<Link5ArchiveDecoder>("kaguya/link5");
