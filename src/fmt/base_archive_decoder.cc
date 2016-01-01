#include "fmt/base_archive_decoder.h"
#include <algorithm>
#include <cmath>
#include "algo/format.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

NamingStrategy BaseArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Child;
}

void BaseArchiveDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

std::unique_ptr<ArchiveMeta> BaseArchiveDecoder::read_meta(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = read_meta_impl(logger, input_file);

    std::string prefix;
    if (naming_strategy() == NamingStrategy::Sibling)
        prefix = input_file.path.stem();
    else if (naming_strategy() == NamingStrategy::FlatSibling)
        prefix = input_file.path.stem();
    else if (naming_strategy() == NamingStrategy::Root)
        prefix = (input_file.path.parent() / input_file.path.stem()).str();

    if (prefix.empty())
        prefix = "unk";

    const auto width = meta->entries.size() > 1
        ? std::max<int>(1, 1 + std::log10(meta->entries.size()))
        : 0;

    int number = 0;
    for (const auto &entry : meta->entries)
    {
        if (!entry->path.str().empty())
            continue;
        entry->path = meta->entries.size() > 1
            ? algo::format("%s_%0*d", prefix.c_str(), width, number++)
            : prefix;
        entry->path.change_extension("dat");
    }

    return meta;
}

std::unique_ptr<io::File> BaseArchiveDecoder::read_file(
    const Logger &logger,
    io::File &input_file,
    const ArchiveMeta &e,
    const ArchiveEntry &m) const
{
    // wrapper reserved for future usage
    return read_file_impl(logger, input_file, e, m);
}
