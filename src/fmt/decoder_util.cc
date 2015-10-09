#include "fmt/decoder_util.h"
#include <set>
#include <stack>
#include "arg_parser.h"
#include "fmt/naming_strategies.h"
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
        auto decoder_to_inspect = decoders_to_inspect.top();
        decoders_to_inspect.pop();
        auto archive_decoder
            = dynamic_cast<const ArchiveDecoder*>(decoder_to_inspect);
        if (!archive_decoder)
            continue;
        for (auto &format : archive_decoder->get_linked_formats())
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
    const FileSaverCallback &recognition_proxy,
    std::shared_ptr<File> original_file,
    std::vector<std::shared_ptr<fmt::IDecoder>> decoders)
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

void fmt::unpack_recursive(
    const std::vector<std::string> &arguments,
    IDecoder &decoder,
    File &file,
    const FileSaver &saver,
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
    static const int max_depth = 10;
    FileSaverCallback recognition_proxy;
    recognition_proxy.set_callback([&](std::shared_ptr<File> original_file)
    {
        util::CallStackKeeper keeper;

        bool bypass_normal_saving = false;
        if (keeper.current_call_depth() <= max_depth)
        {
            bypass_normal_saving = pass_through_decoders(
                recognition_proxy, original_file, decoders);
        }

        if (!bypass_normal_saving)
            saver.save(original_file);
    });

    decoder.unpack(file, recognition_proxy);
}

void fmt::unpack_non_recursive(
    const std::vector<std::string> &arguments,
    IDecoder &decoder,
    File &file,
    const FileSaver &saver)
{
    ArgParser decoder_arg_parser;
    decoder.register_cli_options(decoder_arg_parser);
    decoder_arg_parser.parse(arguments);
    decoder.parse_cli_options(decoder_arg_parser);

    auto archive_decoder = dynamic_cast<ArchiveDecoder*>(&decoder);
    if (archive_decoder)
        archive_decoder->disable_preprocessing();
    decoder.unpack(file, saver);
}
