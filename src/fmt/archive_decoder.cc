#include "fmt/archive_decoder.h"
#include "err.h"
#include "fmt/naming_strategies.h"
#include "util/call_stack_keeper.h"

using namespace au;
using namespace au::fmt;

static bool pass_through_decoders(
    FileSaverCallback &recognition_proxy,
    std::shared_ptr<File> original_file,
    std::vector<IDecoder*> decoders)
{
    for (auto &decoder : decoders)
    {
        FileSaverCallback decoder_proxy(
            [original_file, &recognition_proxy, &decoder]
            (std::shared_ptr<File> converted_file)
        {
            converted_file->name = decoder->naming_strategy()->decorate(
                original_file->name, converted_file->name);
            recognition_proxy.save(converted_file);
        });

        try
        {
            decoder->unpack(*original_file, decoder_proxy);
            return true;
        }
        catch (...)
        {
        }
    }

    return false;
}

ArchiveDecoder::ArchiveDecoder() : nested_decoding_enabled(true)
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
    for (auto &decoder : decoders)
    {
        if (decoder != this)
            decoder->register_cli_options(arg_parser);
    }
}

void ArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    for (auto &decoder : decoders)
    {
        if (decoder != this)
            decoder->parse_cli_options(arg_parser);
    }
}

bool ArchiveDecoder::is_recognized(File &arc_file) const
{
    try
    {
        arc_file.io.seek(0);
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

    // every file should be passed through registered decoders
    static const int max_depth = 10;
    FileSaverCallback recognition_proxy;
    recognition_proxy.set_callback([&](std::shared_ptr<File> original_file)
    {
        util::CallStackKeeper keeper;

        bool bypass_normal_saving = false;
        if (keeper.current_call_depth() <= max_depth && nested_decoding_enabled)
        {
            bypass_normal_saving = pass_through_decoders(
                recognition_proxy, original_file, decoders);
        }

        if (!bypass_normal_saving)
            saver.save(original_file);
    });

    auto meta = read_meta(arc_file);
    if (nested_decoding_enabled)
        preprocess(arc_file, *meta, recognition_proxy);
    for (auto &entry : meta->entries)
    {
        auto output_file = read_file(arc_file, *meta, *entry);
        if (output_file)
            recognition_proxy.save(std::move(output_file));
    }
}

std::unique_ptr<ArchiveMeta> ArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(0);
    return read_meta_impl(arc_file);
}

std::unique_ptr<File> ArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &e, const ArchiveEntry &m) const
{
    // wrapper reserved for future usage
    return read_file_impl(arc_file, e, m);
}

std::vector<std::shared_ptr<File>> ArchiveDecoder::unpack(File &arc_file) const
{
    std::vector<std::shared_ptr<File>> files;
    FileSaverCallback saver([&](std::shared_ptr<File> unpacked_file)
    {
        files.push_back(unpacked_file);
    });
    unpack(arc_file, saver);
    return files;
}

void ArchiveDecoder::preprocess(File &, ArchiveMeta &, const FileSaver &) const
{
}

void ArchiveDecoder::disable_nested_decoding()
{
   nested_decoding_enabled = false;
}

void ArchiveDecoder::add_decoder(IDecoder *decoder)
{
    decoders.push_back(decoder);
}
