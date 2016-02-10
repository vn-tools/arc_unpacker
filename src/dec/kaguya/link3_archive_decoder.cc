#include "dec/kaguya/link3_archive_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK3"_b;

bool Link3ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

int Link3ArchiveDecoder::get_version() const
{
    return 3;
}

static auto _ = dec::register_decoder<Link3ArchiveDecoder>("kaguya/link3");
