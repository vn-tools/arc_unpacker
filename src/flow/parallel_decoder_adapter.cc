#include "flow/parallel_decoder_adapter.h"
#include "fmt/naming_strategies.h"
#include "util/file_from_audio.h"
#include "util/file_from_image.h"
#include "util/virtual_file_system.h"

using namespace au;
using namespace au::flow;

ParallelDecoderAdapter::ParallelDecoderAdapter(
    const BaseParallelUnpackingTask &task,
    const std::shared_ptr<io::File> input_file)
    : task(task), input_file(input_file)
{
}

ParallelDecoderAdapter::~ParallelDecoderAdapter()
{
}

void ParallelDecoderAdapter::visit(const fmt::BaseArchiveDecoder &decoder)
{
    auto input_file = this->input_file;
    auto meta = std::shared_ptr<fmt::ArchiveMeta>(
        decoder.read_meta(task.logger, *input_file));
    task.logger.info(
        "%s: archive contains %d files.\n",
        task.base_name.c_str(),
        meta->entries.size());
    auto decoder_copy = decoder.shared_from_this();

    Logger dummy_logger(task.logger);
    for (const auto &entry : meta->entries)
    {
        const auto new_base_name = fmt::decorate_path(
            decoder.naming_strategy(), task.base_name, entry->path);
        util::VirtualFileSystem::register_file(
            new_base_name,
            [input_file, meta, &entry, dummy_logger, decoder_copy]()
            {
                io::File file_copy(*input_file);
                return
                    static_cast<const fmt::BaseArchiveDecoder&>(*decoder_copy)
                    .read_file(dummy_logger, file_copy, *meta, *entry);
            });
    }

    for (const auto &entry : meta->entries)
    {
        task.unpacker.save_file(
            [input_file, meta, &entry, &decoder](const Logger &logger)
            {
                io::File file_copy(*input_file);
                return decoder.read_file(logger, file_copy, *meta, *entry);
            },
            decoder,
            task);
    }
}

void ParallelDecoderAdapter::visit(const fmt::BaseFileDecoder &decoder)
{
    auto input_file = this->input_file;
    task.unpacker.save_file(
        [input_file, &decoder](const Logger &logger)
        {
            io::File file_copy(*input_file);
            return decoder.decode(logger, file_copy);
        },
        decoder,
        task);
}

void ParallelDecoderAdapter::visit(const fmt::BaseImageDecoder &decoder)
{
    auto input_file = this->input_file;
    task.unpacker.save_file(
        [input_file, &decoder](const Logger &logger)
        {
            io::File file_copy(*input_file);
            return util::file_from_image(
                decoder.decode(logger, file_copy), input_file->path);
        },
        decoder,
        task);
}

void ParallelDecoderAdapter::visit(const fmt::BaseAudioDecoder &decoder)
{
    auto input_file = this->input_file;
    task.unpacker.save_file(
        [input_file, &decoder](const Logger &logger)
        {
            io::File file_copy(*input_file);
            return util::file_from_audio(
                decoder.decode(logger, file_copy), input_file->path);
        },
        decoder,
        task);
}
