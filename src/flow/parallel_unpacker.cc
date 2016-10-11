// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "flow/parallel_unpacker.h"
#include <chrono>
#include <set>
#include <stack>
#include "algo/format.h"
#include "dec/idecoder.h"
#include "err.h"
#include "flow/parallel_decoder_adapter.h"

using namespace au;
using namespace au::flow;

static const auto max_depth = 10;
static int task_count = 0;
static std::mutex mutex;

namespace
{
    struct DecodeInputFileTask final : public BaseParallelUnpackingTask
    {
        DecodeInputFileTask(
            ParallelTaskContext &task_context,
            const TaskSourceType source_type,
            const io::path &base_name,
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
            const std::set<std::string> &decoders_to_check,
            const InputFileFactory file_factory);

        bool work() const override;

        const InputFileFactory file_factory;
    };

    struct ProcessOutputFileTask final : public BaseParallelUnpackingTask
    {
        ProcessOutputFileTask(
            ParallelTaskContext &task_context,
            const TaskSourceType source_type,
            const io::path &base_name,
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
            const std::set<std::string> &decoders_to_check,
            const std::shared_ptr<io::File> input_file,
            const DecoderFileFactory file_factory,
            const std::shared_ptr<const dec::IDecoder> origin_decoder,
            const std::string &target_name);

        bool work() const override;

        const std::shared_ptr<io::File> input_file;
        const DecoderFileFactory file_factory;
        const std::shared_ptr<const dec::IDecoder> origin_decoder;
        const std::string target_name;
    };
}

static bool save(
    const BaseParallelUnpackingTask &task, std::shared_ptr<io::File> file)
{
    try
    {
        const auto full_path
            = task.task_context.unpacker_context.file_saver.save(file);
        task.logger.success("saved to %s\n", full_path.c_str());
        task.logger.flush();
        return true;
    }
    catch (const err::IoError &e)
    {
        task.logger.err(
            "error saving (%s)\n", e.what() ? e.what() : "unknown error");
        task.logger.flush();
        return false;
    }
}

static std::set<std::string> collect_linked_decoders(
    const dec::IDecoder &base_decoder, const dec::Registry &registry)
{
    std::set<std::string> known_formats;
    std::vector<std::shared_ptr<dec::IDecoder>> linked_decoders;
    std::stack<const dec::IDecoder*> decoders_to_inspect;
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
    return std::set<std::string>(known_formats.begin(), known_formats.end());
}

static std::shared_ptr<dec::IDecoder> guess_decoder(
    const BaseParallelUnpackingTask &task,
    const std::set<std::string> &decoders_to_check,
    io::File &file,
    const TaskSourceType source_type)
{
    task.logger.info(
        "guessing decoder among %d decoders...\n", decoders_to_check.size());

    std::map<std::string, std::shared_ptr<dec::IDecoder>> matching_decoders;
    for (const auto &name : decoders_to_check)
    {
        const auto current_decoder
            = task.task_context.unpacker_context.registry.create_decoder(name);
        if (current_decoder->is_recognized(file))
            matching_decoders[name] = std::move(current_decoder);
    }

    if (matching_decoders.size() == 1)
    {
        task.logger.success(
            "recognized as %s.\n", matching_decoders.begin()->first.c_str());
        return matching_decoders.begin()->second;
    }

    if (matching_decoders.empty())
    {
        if (source_type == TaskSourceType::NestedDecoding)
        {
            task.logger.info("not recognized by any decoder.\n");
        }
        else
        {
            task.logger.err("not recognized by any decoder.\n");
        }
        return nullptr;
    }

    if (source_type == TaskSourceType::NestedDecoding)
    {
        task.logger.warn("file was recognized by multiple decoders.\n");
    }
    else
    {
        task.logger.warn("file was recognized by multiple decoders.\n");
        for (const auto &it : matching_decoders)
            task.logger.warn("- " + it.first + "\n");
        task.logger.warn("Please provide --dec and proceed manually.\n");
    }
    return nullptr;
}

ParallelUnpackerContext::ParallelUnpackerContext(
    const Logger &logger,
    const IFileSaver &file_saver,
    const dec::Registry &registry,
    const bool enable_nested_decoding,
    const std::vector<std::string> &arguments,
    const std::set<std::string> &decoders_to_check) :
        logger(logger),
        file_saver(file_saver),
        registry(registry),
        enable_nested_decoding(enable_nested_decoding),
        arguments(arguments),
        decoders_to_check(decoders_to_check)
{
}

ParallelTaskContext::ParallelTaskContext(
    ParallelUnpacker &unpacker,
    const ParallelUnpackerContext &unpacker_context,
    TaskScheduler &task_scheduler) :
        unpacker(unpacker),
        unpacker_context(unpacker_context),
        task_scheduler(task_scheduler)
{
}

BaseParallelUnpackingTask::BaseParallelUnpackingTask(
    ParallelTaskContext &task_context,
    const TaskSourceType source_type,
    const io::path &base_name,
    const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
    const std::set<std::string> &decoders_to_check) :
        logger(task_context.unpacker_context.logger),
        task_context(task_context),
        source_type(source_type),
        base_name(base_name),
        parent_task(parent_task),
        decoders_to_check(decoders_to_check)
{
    mutex.lock();
    const auto task_id = task_count++;
    mutex.unlock();
    logger.set_prefix(
        algo::format("[task %d] %s: ", task_id, base_name.c_str()));
}

size_t BaseParallelUnpackingTask::get_depth() const
{
    auto depth = 0;
    auto task = parent_task;
    while (task != nullptr)
    {
        ++depth;
        task = task->parent_task;
    }
    return depth;
}

void BaseParallelUnpackingTask::save_file(
    const std::shared_ptr<io::File> input_file,
    const DecoderFileFactory file_factory,
    const dec::BaseDecoder &origin_decoder,
    const std::string &target_name) const
{
    task_context.task_scheduler.push_front(
        std::make_shared<ProcessOutputFileTask>(
            task_context,
            source_type,
            base_name,
            shared_from_this(),
            source_type == TaskSourceType::InitialUserInput
                ? std::set<std::string>() : decoders_to_check,
            input_file,
            file_factory,
            origin_decoder.shared_from_this(),
            target_name));
}

DecodeInputFileTask::DecodeInputFileTask(
    ParallelTaskContext &task_context,
    const TaskSourceType source_type,
    const io::path &base_name,
    const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
    const std::set<std::string> &decoders_to_check,
    const InputFileFactory file_factory) :
        BaseParallelUnpackingTask(
            task_context,
            source_type,
            base_name,
            parent_task,
            decoders_to_check),
        file_factory(file_factory)
{
}

bool DecodeInputFileTask::work() const
{
    std::shared_ptr<io::File> input_file;
    try
    {
        input_file = file_factory();
        if (!input_file)
        {
            logger.err("no input file (?)\n");
            return false;
        }
    }
    catch (const std::exception &e)
    {
        logger.err("unknown input file: error opening (%s)\n", e.what());
        return false;
    }

    try
    {
        logger.info("initial recognition...\n");

        const auto decoder = guess_decoder(
            *this, decoders_to_check, *input_file, source_type);

        if (!decoder)
        {
            return source_type == TaskSourceType::NestedDecoding
                ? save(*this, input_file)
                : false;
        }

        ArgParser decoder_arg_parser;
        const auto decorators = decoder->get_arg_parser_decorators();
        for (const auto &decorator : decorators)
            decorator.register_cli_options(decoder_arg_parser);
        decoder_arg_parser.parse(task_context.unpacker_context.arguments);
        for (const auto &decorator : decorators)
            decorator.parse_cli_options(decoder_arg_parser);

        ParallelDecoderAdapter adapter(shared_from_this(), input_file);
        decoder->accept(adapter);
        return true;
    }
    catch (const std::exception &e)
    {
        logger.err("recognition finished with errors:\n%s\n", e.what());
        if (source_type == TaskSourceType::NestedDecoding)
            save(*this, input_file);
        return false;
    }
}

ProcessOutputFileTask::ProcessOutputFileTask(
    ParallelTaskContext &task_context,
    const TaskSourceType source_type,
    const io::path &base_name,
    const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
    const std::set<std::string> &decoders_to_check,
    const std::shared_ptr<io::File> input_file,
    const DecoderFileFactory file_factory,
    const std::shared_ptr<const dec::IDecoder> origin_decoder,
    const std::string &target_name) :
        BaseParallelUnpackingTask(
            task_context,
            source_type,
            base_name,
            parent_task,
            decoders_to_check),
        input_file(input_file),
        file_factory(file_factory),
        origin_decoder(origin_decoder),
        target_name(target_name)
{
}

bool ProcessOutputFileTask::work() const
{
    logger.info(
        target_name.empty()
            ? "decoding...\n"
            : "decoding \"%s\"...\n",
        target_name.c_str());
    if (!input_file)
    {
        logger.err("error obtaining input file!\n");
        return false;
    }

    io::File input_file_copy(*input_file);
    std::shared_ptr<io::File> output_file;
    try
    {
        output_file = file_factory(input_file_copy, logger);
        if (!output_file)
        {
            logger.info(
                target_name.empty()
                    ? "decoding of \"%s\" ommitted.\n"
                    : "decoding ommitted.\n",
                target_name.c_str());
            return false;
        }
    }
    catch (const std::exception &e)
    {
        if (target_name.empty())
        {
            logger.err("error decoding (%s)\n", e.what());
        }
        else
        {
            logger.err(
                "error decoding \"%s\" (%s)\n", target_name.c_str(), e.what());
        }
        if (source_type == TaskSourceType::NestedDecoding)
            save(*this, input_file);
        return false;
    }

    logger.info(
        target_name.empty()
            ? "decoding finished\n"
            : "decoding of \"%s\" finished.\n",
        target_name.c_str());

    const auto naming_strategy = origin_decoder->naming_strategy();
    output_file->path = algo::apply_naming_strategy(
        naming_strategy, base_name, output_file->path);

    if (!task_context.unpacker_context.enable_nested_decoding)
        return save(*this, output_file);

    auto linked_decoders = collect_linked_decoders(
        *origin_decoder, task_context.unpacker_context.registry);
    linked_decoders.insert(
        decoders_to_check.begin(), decoders_to_check.end());

    if (linked_decoders.empty())
        return save(*this, output_file);

    if (get_depth() >= max_depth)
    {
        logger.warn("cycle detected.\n");
        return save(*this, output_file);
    }

    task_context.task_scheduler.push_front(
        std::make_shared<DecodeInputFileTask>(
            task_context,
            TaskSourceType::NestedDecoding,
            output_file->path,
            shared_from_this(),
            linked_decoders,
            [=]() { return output_file; }));

    return true;
}

struct ParallelUnpacker::Priv final
{
    Priv(
        ParallelUnpacker &unpacker,
        const ParallelUnpackerContext &unpacker_context);

    const ParallelUnpackerContext &unpacker_context;
    TaskScheduler task_scheduler;
    ParallelTaskContext task_context;
};

ParallelUnpacker::Priv::Priv(
    ParallelUnpacker &unpacker,
    const ParallelUnpackerContext &unpacker_context) :
        unpacker_context(unpacker_context),
        task_context(unpacker, unpacker_context, task_scheduler)
{
}

ParallelUnpacker::ParallelUnpacker(
    const ParallelUnpackerContext &unpacker_context)
        : p(new Priv(*this, unpacker_context))
{
}

ParallelUnpacker::~ParallelUnpacker()
{
}

void ParallelUnpacker::add_input_file(
    const io::path &base_name, const InputFileFactory file_factory)
{
    p->task_scheduler.push_back(
        std::make_shared<DecodeInputFileTask>(
            p->task_context,
            TaskSourceType::InitialUserInput,
            base_name,
            nullptr,
            p->unpacker_context.decoders_to_check,
            file_factory));
}

bool ParallelUnpacker::run(const size_t thread_count)
{
    const auto begin = std::chrono::steady_clock::now();
    const auto results = p->task_scheduler.run(thread_count);
    const auto end = std::chrono::steady_clock::now();
    const auto diff
        = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    Logger logger(p->unpacker_context.logger);

    logger.log(
        Logger::MessageType::Summary,
        "Executed %d tasks in %.02fs (",
        results.success_count + results.error_count,
        diff.count() / 1000.0);

    if (results.error_count > 0)
    {
        logger.set_color(Logger::Color::Red);
        logger.log(
            Logger::MessageType::Summary,
            "%d problem%s",
            results.error_count,
            results.error_count == 1 ? "" : "s");
        logger.set_color(Logger::Color::Original);
        logger.log(Logger::MessageType::Summary, ", ");
    }

    logger.log(
        Logger::MessageType::Summary,
        "%d saved files)\n",
        p->unpacker_context.file_saver.get_saved_file_count());

    return results.error_count == 0;
}
