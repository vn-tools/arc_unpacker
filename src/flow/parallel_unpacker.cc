#include "flow/parallel_unpacker.h"
#include <set>
#include <stack>
#include "algo/format.h"
#include "err.h"
#include "file_saver.h"
#include "flow/parallel_decoder_adapter.h"
#include "fmt/idecoder.h"

using namespace au;
using namespace au::flow;

static const auto max_depth = 10;
static int task_count = 0;
static std::mutex mutex;

namespace
{
    enum class SourceType : u8
    {
        InitialUserInput,
        NestedDecoding,
    };

    struct DecodeInputFileTask final : public BaseParallelUnpackingTask
    {
        DecodeInputFileTask(
            ParallelUnpacker &unpacker,
            TaskScheduler &task_scheduler,
            const ParallelUnpackerContext &unpacker_context,
            const size_t depth,
            const io::path &base_name,
            const SourceType source_type,
            const std::vector<std::string> &available_decoders,
            const std::function<std::shared_ptr<io::File>()> file_provider);

        bool work() const override;

        const SourceType source_type;
        const std::vector<std::string> available_decoders;
        const std::function<std::shared_ptr<io::File>()> file_provider;
    };

    struct ProcessOutputFileTask final : public BaseParallelUnpackingTask
    {
        ProcessOutputFileTask(
            ParallelUnpacker &unpacker,
            TaskScheduler &task_scheduler,
            const ParallelUnpackerContext &unpacker_context,
            const size_t depth,
            const io::path &base_name,
            const std::function<std::shared_ptr<io::File>()> file_factory,
            const std::shared_ptr<const fmt::IDecoder> origin_decoder,
            const io::path &origin_path);

        bool work() const override;

        const std::function<std::shared_ptr<io::File>()> file_factory;
        const std::shared_ptr<const fmt::IDecoder> origin_decoder;
        const io::path origin_path;
    };
}

static bool save(
    const BaseParallelUnpackingTask &task, std::shared_ptr<io::File> file)
{
    try
    {
        const auto full_path = task.unpacker_context.file_saver.save(file);
        task.logger.success(
            "%s: saved to %s\n", file->path.c_str(), full_path.c_str());
        task.logger.flush();
        return true;
    }
    catch (err::IoError &e)
    {
        task.logger.err(
            "%s: error saving (%s)\n",
            file->path.c_str(),
            e.what() ? e.what() : "unknown error");
        task.logger.flush();
        return false;
    }
}

static std::vector<std::string> collect_linked_decoders(
    const fmt::IDecoder &base_decoder, const fmt::Registry &registry)
{
    std::set<std::string> known_formats;
    std::vector<std::shared_ptr<fmt::IDecoder>> linked_decoders;
    std::stack<const fmt::IDecoder*> decoders_to_inspect;
    decoders_to_inspect.push(&base_decoder);
    while (!decoders_to_inspect.empty())
    {
        const auto decoder_to_inspect = decoders_to_inspect.top();
        decoders_to_inspect.pop();
        for (const auto &format : decoder_to_inspect->get_linked_formats())
        {
            if (known_formats.find(format) != known_formats.end())
                continue;
            known_formats.insert(format);
            auto linked_decoder = registry.create_decoder(format);
            decoders_to_inspect.push(linked_decoder.get());
            linked_decoders.push_back(std::move(linked_decoder));
        }
    }
    return std::vector<std::string>(known_formats.begin(), known_formats.end());
}

static std::shared_ptr<fmt::IDecoder> guess_decoder(
    const Logger &logger,
    const fmt::Registry &registry,
    const std::vector<std::string> &available_decoders,
    io::File &file,
    const SourceType source_type)
{
    logger.info(
        "%s: guessing decoder among %d decoders...\n",
        file.path.c_str(),
        available_decoders.size());

    std::map<std::string, std::shared_ptr<fmt::IDecoder>> matching_decoders;
    for (const auto &name : available_decoders)
    {
        auto current_decoder = registry.create_decoder(name);
        if (current_decoder->is_recognized(file))
            matching_decoders[name] = std::move(current_decoder);
    }

    if (matching_decoders.size() == 1)
    {
        logger.success(
            "%s: recognized as %s.\n",
            file.path.c_str(),
            matching_decoders.begin()->first.c_str());
        return matching_decoders.begin()->second;
    }

    if (matching_decoders.empty())
    {
        if (source_type == SourceType::NestedDecoding)
        {
            logger.info(
                "%s: not recognized by any decoder.\n", file.path.c_str());
        }
        else
        {
            logger.err(
                "%s: not recognized by any decoder.\n", file.path.c_str());
        }
        return nullptr;
    }

    if (source_type == SourceType::NestedDecoding)
    {
        logger.warn(
            "%s: file was recognized by multiple decoders.\n",
            file.path.c_str());
    }
    else
    {
        logger.warn(
            "%s: file was recognized by multiple decoders.\n",
            file.path.c_str());
        for (const auto &it : matching_decoders)
            logger.warn("- " + it.first + "\n");
        logger.warn("Please provide --fmt and proceed manually.\n");
    }
    return nullptr;
}

ParallelUnpackerContext::ParallelUnpackerContext(
    const Logger &logger,
    const FileSaver &file_saver,
    const fmt::Registry &registry,
    const bool enable_nested_decoding,
    const std::vector<std::string> &arguments,
    const std::vector<std::string> &available_decoders) :
        logger(logger),
        file_saver(file_saver),
        registry(registry),
        enable_nested_decoding(enable_nested_decoding),
        arguments(arguments),
        available_decoders(available_decoders)
{
}

BaseParallelUnpackingTask::BaseParallelUnpackingTask(
    ParallelUnpacker &unpacker,
    TaskScheduler &task_scheduler,
    const ParallelUnpackerContext &unpacker_context,
    const size_t depth,
    const io::path &base_name) :
        logger(unpacker_context.logger),
        unpacker(unpacker),
        task_scheduler(task_scheduler),
        unpacker_context(unpacker_context),
        depth(depth),
        base_name(base_name)
{
    mutex.lock();
    task_id = task_count++;
    mutex.unlock();
    logger.set_prefix(algo::format("[task %d] ", task_id));
}

DecodeInputFileTask::DecodeInputFileTask(
    ParallelUnpacker &unpacker,
    TaskScheduler &task_scheduler,
    const ParallelUnpackerContext &unpacker_context,
    const size_t depth,
    const io::path &base_name,
    const SourceType source_type,
    const std::vector<std::string> &available_decoders,
    const std::function<std::shared_ptr<io::File>()> file_provider) :
        BaseParallelUnpackingTask(
            unpacker, task_scheduler, unpacker_context, depth, base_name),
        source_type(source_type),
        available_decoders(available_decoders),
        file_provider(file_provider)
{
}

bool DecodeInputFileTask::work() const
{
    std::shared_ptr<io::File> input_file;
    try
    {
        input_file = file_provider();
        if (!input_file)
        {
            logger.err("No input file(?)\n");
            return false;
        }
    }
    catch (std::exception &e)
    {
        logger.err("unknown input file: error opening (%s)\n", e.what());
        return false;
    }

    try
    {
        logger.info("%s: initial recognition...\n", input_file->path.c_str());

        const auto decoder = guess_decoder(
            logger,
            unpacker_context.registry,
            available_decoders,
            *input_file,
            source_type);

        if (!decoder)
        {
            return source_type == SourceType::NestedDecoding
                ? save(*this, input_file)
                : false;
        }

        ArgParser decoder_arg_parser;
        decoder->register_cli_options(decoder_arg_parser);
        decoder_arg_parser.parse(unpacker_context.arguments);
        decoder->parse_cli_options(decoder_arg_parser);

        ParallelDecoderAdapter adapter(*this, input_file);
        decoder->accept(adapter);
        return true;
    }
    catch (std::exception &e)
    {
        logger.err(
            "%s: recognition finished with errors (%s)\n",
            input_file->path.c_str(),
            e.what());
        return false;
    }
}

ProcessOutputFileTask::ProcessOutputFileTask(
    ParallelUnpacker &unpacker,
    TaskScheduler &task_scheduler,
    const ParallelUnpackerContext &unpacker_context,
    const size_t depth,
    const io::path &base_name,
    const std::function<std::shared_ptr<io::File>()> file_factory,
    const std::shared_ptr<const fmt::IDecoder> origin_decoder,
    const io::path &origin_path) :
        BaseParallelUnpackingTask(
            unpacker, task_scheduler, unpacker_context, depth, base_name),
        file_factory(file_factory),
        origin_decoder(origin_decoder),
        origin_path(origin_path)
{
}

bool ProcessOutputFileTask::work() const
{
    logger.info("%s: decoding file...\n", origin_path.c_str());
    std::shared_ptr<io::File> output_file;
    try
    {
        output_file = file_factory();
        if (!output_file)
            return false;
    }
    catch (std::exception &e)
    {
        logger.err("%s: error decoding (%s)\n", origin_path.c_str(), e.what());
        return false;
    }
    logger.info("%s: decoding finished.\n", origin_path.c_str());

    const auto naming_strategy = origin_decoder->naming_strategy();
    try
    {
        output_file->path = decorate_path(
            naming_strategy, base_name, output_file->path);

        if (!unpacker_context.enable_nested_decoding)
            return save(*this, output_file);

        const auto linked_decoders = collect_linked_decoders(
            *origin_decoder, unpacker_context.registry);

        if (linked_decoders.empty())
            return save(*this, output_file);

        if (depth >= max_depth)
        {
            logger.warn("%s: cycle detected.\n", origin_path.c_str());
            return save(*this, output_file);
        }

        task_scheduler.push_front(
            std::make_unique<DecodeInputFileTask>(
                unpacker,
                task_scheduler,
                unpacker_context,
                depth + 1,
                output_file->path,
                SourceType::NestedDecoding,
                linked_decoders,
                [=]() { return output_file; }));

        return true;
    }
    catch (std::exception &e)
    {
        logger.err(
            "Can't process %s: %s\n", output_file->path.c_str(), e.what());
        return false;
    }
}

struct ParallelUnpacker::Priv final
{
    Priv(const ParallelUnpackerContext &context);
    const ParallelUnpackerContext &context;
    TaskScheduler task_scheduler;
};

ParallelUnpacker::Priv::Priv(const ParallelUnpackerContext &context)
    : context(context)
{
}

ParallelUnpacker::ParallelUnpacker(const ParallelUnpackerContext &context)
    : p(new Priv(context))
{
}

ParallelUnpacker::~ParallelUnpacker()
{
}

void ParallelUnpacker::add_input_file(
    const io::path &base_name,
    const std::function<std::shared_ptr<io::File>()> file_factory)
{
    p->task_scheduler.push_back(
        std::make_unique<DecodeInputFileTask>(
            *this,
            p->task_scheduler,
            p->context,
            0,
            base_name,
            SourceType::InitialUserInput,
            p->context.available_decoders,
            file_factory));
}

void ParallelUnpacker::save_file(
    const std::function<std::shared_ptr<io::File>()> file_factory,
    const std::shared_ptr<const fmt::IDecoder> origin_decoder,
    const io::path &origin_path,
    const BaseParallelUnpackingTask &origin_task)
{
    p->task_scheduler.push_front(
        std::make_unique<ProcessOutputFileTask>(
            *this,
            p->task_scheduler,
            p->context,
            origin_task.depth + 1,
            origin_task.base_name,
            file_factory,
            origin_decoder,
            origin_path));
}

bool ParallelUnpacker::run(const size_t thread_count)
{
    return p->task_scheduler.run(thread_count);
}
