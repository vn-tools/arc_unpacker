#include "fmt/archive_decoder.h"
#include "algo/format.h"
#include "err.h"
#include "util/virtual_file_system.h"

using namespace au;
using namespace au::fmt;

ArchiveDecoder::ArchiveDecoder() : preprocessing_disabled(false)
{
}

IDecoder::NamingStrategy ArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Child;
}

void ArchiveDecoder::unpack(
    const Logger &logger,
    io::File &input_file,
    const FileSaver &file_saver) const
{
    if (!is_recognized(input_file))
        throw err::RecognitionError();

    size_t error_count = 0;
    auto meta = read_meta(logger, input_file);

    for (const auto &entry : meta->entries)
    {
        util::VirtualFileSystem::register_file(entry->path, [&]()
            {
                return read_file(logger, input_file, *meta, *entry);
            });
    }

    if (!preprocessing_disabled)
        preprocess(logger, input_file, *meta, file_saver);
    for (auto &entry : meta->entries)
    {
        try
        {
            auto output_file = read_file(logger, input_file, *meta, *entry);
            if (output_file)
                file_saver.save(std::move(output_file));
        }
        catch (std::exception &e)
        {
            ++error_count;
            logger.err("Can't unpack %s: %s\n", entry->path.c_str(), e.what());
        }
    }

    for (const auto &entry : meta->entries)
        util::VirtualFileSystem::unregister_file(entry->path);

    if (error_count)
    {
        throw err::GeneralError(
            algo::format("%d files couldn't be unpacked.", error_count));
    }
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

void ArchiveDecoder::preprocess(
    const Logger &logger, io::File &, ArchiveMeta &, const FileSaver &) const
{
}

void ArchiveDecoder::disable_preprocessing()
{
   preprocessing_disabled = true;
}

std::vector<std::string> ArchiveDecoder::get_linked_formats() const
{
    return {};
}
