#include "fmt/archive_decoder.h"
#include "err.h"
#include "fmt/naming_strategies.h"
#include "log.h"
#include "util/format.h"

using namespace au;
using namespace au::fmt;

ArchiveDecoder::ArchiveDecoder() : preprocessing_disabled(false)
{
}

ArchiveDecoder::~ArchiveDecoder()
{
}

std::unique_ptr<INamingStrategy> ArchiveDecoder::naming_strategy() const
{
    return std::make_unique<ChildNamingStrategy>();
}

void ArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
}

void ArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
}

bool ArchiveDecoder::is_recognized(File &arc_file) const
{
    try
    {
        arc_file.stream.seek(0);
        return is_recognized_impl(arc_file);
    }
    catch (...)
    {
        return false;
    }
}

void ArchiveDecoder::unpack(File &arc_file, const FileSaver &saver) const
{
    if (!is_recognized(arc_file))
        throw err::RecognitionError();

    size_t error_count = 0;
    auto meta = read_meta(arc_file);
    if (!preprocessing_disabled)
        preprocess(arc_file, *meta, saver);
    for (auto &entry : meta->entries)
    {
        try
        {
            auto output_file = read_file(arc_file, *meta, *entry);
            if (output_file)
                saver.save(std::move(output_file));
        }
        catch (std::exception &e)
        {
            ++error_count;
            Log.err(
                "Can't unpack %s: %s\n",
                entry->name.c_str(),
                e.what());
        }
    }
    if (error_count)
    {
        throw err::GeneralError(
            util::format("%d files couldn't be unpacked.", error_count));
    }
}

std::unique_ptr<ArchiveMeta> ArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.stream.seek(0);
    return read_meta_impl(arc_file);
}

std::unique_ptr<File> ArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &e, const ArchiveEntry &m) const
{
    // wrapper reserved for future usage
    return read_file_impl(arc_file, e, m);
}

void ArchiveDecoder::preprocess(File &, ArchiveMeta &, const FileSaver &) const
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
