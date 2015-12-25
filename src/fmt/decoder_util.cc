#include "fmt/decoder_util.h"
#include <set>
#include <stack>
#include "arg_parser.h"
#include "fmt/archive_decoder.h"
#include "util/call_stack_keeper.h"

using namespace au;
using namespace au::fmt;

static std::vector<std::shared_ptr<IDecoder>> collect_linked_decoders(
    const IDecoder &base_decoder, const Registry &registry)
{
    std::set<std::string> known_formats;
    std::vector<std::shared_ptr<IDecoder>> linked_decoders;
    std::stack<const IDecoder*> decoders_to_inspect;
    decoders_to_inspect.push(&base_decoder);
    while (!decoders_to_inspect.empty())
    {
        const auto decoder_to_inspect = decoders_to_inspect.top();
        decoders_to_inspect.pop();
        const auto archive_decoder
            = dynamic_cast<const ArchiveDecoder*>(decoder_to_inspect);
        if (!archive_decoder)
            continue;
        for (const auto &format : archive_decoder->get_linked_formats())
        {
            if (known_formats.find(format) != known_formats.end())
                continue;
            known_formats.insert(format);
            auto linked_decoder = registry.create_decoder(format);
            decoders_to_inspect.push(linked_decoder.get());
            linked_decoders.push_back(std::move(linked_decoder));
        }
    }
    return linked_decoders;
}

static bool pass_through_decoders(
    const Logger &logger,
    const FileSaverCallback &recognition_proxy,
    const std::shared_ptr<io::File> original_file,
    const std::vector<std::shared_ptr<fmt::IDecoder>> &decoders)
{
    for (const auto &decoder : decoders)
    {
        FileSaverCallback decoder_proxy(
            [original_file, &recognition_proxy, &decoder]
            (const std::shared_ptr<io::File> converted_file)
        {
            converted_file->path = decorate_path(
                decoder->naming_strategy(),
                original_file->path,
                converted_file->path);
            recognition_proxy.save(converted_file);
        });

        try
        {
            decoder->unpack(logger, *original_file, decoder_proxy);
            return true;
        }
        catch (...)
        {
        }
    }

    return false;
}

io::path fmt::decorate_path(
    const IDecoder::NamingStrategy strategy,
    const io::path &parent_path,
    const io::path &current_path)
{
    if (strategy == IDecoder::NamingStrategy::Root)
        return current_path;

    if (strategy == IDecoder::NamingStrategy::Child)
    {
        if (parent_path.str() == "")
            return current_path;
        return parent_path / current_path;
    }

    if (strategy == IDecoder::NamingStrategy::Sibling)
    {
        if (parent_path.str() == "")
            return current_path;
        return parent_path.parent() / current_path;
    }

    throw std::logic_error("Invalid naming strategy");
}

void fmt::unpack_recursive(
    const Logger &logger,
    const std::vector<std::string> &arguments,
    IDecoder &decoder,
    io::File &file,
    const FileSaver &file_saver,
    const Registry &registry)
{
    ArgParser decoder_arg_parser;
    auto decoders = collect_linked_decoders(decoder, registry);

    for (auto &decoder : decoders)
        decoder->register_cli_options(decoder_arg_parser);
    decoder.register_cli_options(decoder_arg_parser);
    decoder_arg_parser.parse(arguments);
    for (auto &decoder : decoders)
        decoder->parse_cli_options(decoder_arg_parser);
    decoder.parse_cli_options(decoder_arg_parser);

    // every file should be passed through registered decoders
    util::CallStackKeeper keeper;
    FileSaverCallback recognition_proxy;
    recognition_proxy.set_callback([&](std::shared_ptr<io::File> original_file)
    {
        bool bypass_normal_saving = false;
        if (!keeper.recursion_limit_reached())
        {
            keeper.recurse([&]()
                {
                    bypass_normal_saving = pass_through_decoders(
                        logger, recognition_proxy, original_file, decoders);
                });
        }

        if (!bypass_normal_saving)
            file_saver.save(original_file);
    });

    decoder.unpack(logger, file, recognition_proxy);
}

void fmt::unpack_non_recursive(
    const Logger &logger,
    const std::vector<std::string> &arguments,
    IDecoder &decoder,
    io::File &file,
    const FileSaver &file_saver)
{
    ArgParser decoder_arg_parser;
    decoder.register_cli_options(decoder_arg_parser);
    decoder_arg_parser.parse(arguments);
    decoder.parse_cli_options(decoder_arg_parser);

    auto archive_decoder = dynamic_cast<ArchiveDecoder*>(&decoder);
    if (archive_decoder)
        archive_decoder->disable_preprocessing();
    decoder.unpack(logger, file, file_saver);
}
