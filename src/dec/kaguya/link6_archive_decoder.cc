#include "dec/kaguya/link6_archive_decoder.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK6"_b;

bool Link6ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

int Link6ArchiveDecoder::get_version() const
{
    return 6;
}

static auto _ = dec::register_decoder<Link6ArchiveDecoder>("kaguya/link6");
