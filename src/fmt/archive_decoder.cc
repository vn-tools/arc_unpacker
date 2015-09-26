#include "fmt/archive_decoder.h"
#include "err.h"

using namespace au;
using namespace au::fmt;

static const int max_depth = 10;
static int depth = 0;

namespace
{
    struct DepthKeeper final
    {
        DepthKeeper();
        ~DepthKeeper();
    };
}

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
            converted_file->name =
                FileNameDecorator::decorate(
                    decoder->get_file_naming_strategy(),
                    original_file->name,
                    converted_file->name);
            recognition_proxy.save(converted_file);
        });

        try
        {
            decoder->unpack(*original_file, decoder_proxy, true);
            return true;
        }
        catch (...)
        {
        }
    }

    return false;
}

DepthKeeper::DepthKeeper()
{
    depth++;
}

DepthKeeper::~DepthKeeper()
{
    depth--;
}

ArchiveDecoder::~ArchiveDecoder()
{
}

FileNamingStrategy ArchiveDecoder::get_file_naming_strategy() const
{
    return FileNamingStrategy::Child;
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

bool ArchiveDecoder::is_recognized(File &file) const
{
    try
    {
        file.io.seek(0);
        return is_recognized_internal(file);
    }
    catch (...)
    {
        return false;
    }
}

void ArchiveDecoder::unpack(
    File &file, FileSaver &file_saver, bool recurse) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();

    // every file should be passed through registered decoders
    FileSaverCallback recognition_proxy;
    recognition_proxy.set_callback([&](std::shared_ptr<File> original_file)
    {
        DepthKeeper keeper;

        bool save_normally;
        if (depth > max_depth || !recurse)
        {
            save_normally = true;
        }
        else
        {
            save_normally = !pass_through_decoders(
                recognition_proxy, original_file, decoders);
        }

        if (save_normally)
            file_saver.save(original_file);
    });

    file.io.seek(0);
    return unpack_internal(file, recognition_proxy);
}

void ArchiveDecoder::add_decoder(IDecoder *decoder)
{
    decoders.push_back(decoder);
}
