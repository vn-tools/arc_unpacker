#include "fmt/archive_decoder.h"
#include "algo/format.h"
#include "err.h"
#include "fmt/idecoder_visitor.h"

using namespace au;
using namespace au::fmt;

IDecoder::NamingStrategy ArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Child;
}

void ArchiveDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

std::unique_ptr<ArchiveMeta> ArchiveDecoder::read_meta(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = read_meta_impl(logger, input_file);

    std::string prefix;
    if (naming_strategy() == IDecoder::NamingStrategy::Sibling)
        prefix = input_file.path.stem();
    else if (naming_strategy() == IDecoder::NamingStrategy::Root)
        prefix = input_file.path.str();

    if (prefix.empty())
        prefix = "unk";

    int number = 0;
    for (const auto &entry : meta->entries)
    {
        if (!entry->path.str().empty())
            continue;
        entry->path = meta->entries.size() > 1
            ? algo::format("%s_%03d", prefix.c_str(), number++)
            : prefix;
    }

    return meta;
}

std::unique_ptr<io::File> ArchiveDecoder::read_file(
    const Logger &logger,
    io::File &input_file,
    const ArchiveMeta &e,
    const ArchiveEntry &m) const
{
    // wrapper reserved for future usage
    return read_file_impl(logger, input_file, e, m);
}
