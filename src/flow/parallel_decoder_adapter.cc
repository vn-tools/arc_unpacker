#include "flow/parallel_decoder_adapter.h"
#include "flow/vfs_bridge.h"
#include "fmt/naming_strategies.h"
#include "util/file_from_audio.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::flow;

ParallelDecoderAdapter::ParallelDecoderAdapter(
    const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
    const std::shared_ptr<io::File> input_file)
    : parent_task(parent_task), input_file(input_file)
{
}

ParallelDecoderAdapter::~ParallelDecoderAdapter()
{
}

void ParallelDecoderAdapter::visit(const fmt::BaseArchiveDecoder &decoder)
{
    auto input_file = this->input_file;
    auto meta = std::shared_ptr<fmt::ArchiveMeta>(
        decoder.read_meta(parent_task->logger, *input_file));
    parent_task->logger.info(
        "%s: archive contains %d files.\n",
        parent_task->base_name.c_str(),
        meta->entries.size());

    const auto vfs_bridge = std::make_shared<VirtualFileSystemBridge>(
        parent_task->logger,
        decoder,
        meta,
        input_file,
        parent_task->base_name);

    for (const auto &entry : meta->entries)
    {
        parent_task->unpacker.save_file(
            [input_file, meta, &entry, &decoder, vfs_bridge]
            (const Logger &logger)
            {
                io::File file_copy(*input_file);
                return decoder.read_file(logger, file_copy, *meta, *entry);
            },
            decoder,
            parent_task);
    }
}

void ParallelDecoderAdapter::visit(const fmt::BaseFileDecoder &decoder)
{
    auto input_file = this->input_file;
    parent_task->unpacker.save_file(
        [input_file, &decoder](const Logger &logger)
        {
            io::File file_copy(*input_file);
            return decoder.decode(logger, file_copy);
        },
        decoder,
        parent_task);
}

void ParallelDecoderAdapter::visit(const fmt::BaseImageDecoder &decoder)
{
    auto input_file = this->input_file;
    parent_task->unpacker.save_file(
        [input_file, &decoder](const Logger &logger)
        {
            io::File file_copy(*input_file);
            return util::file_from_image(
                decoder.decode(logger, file_copy), input_file->path);
        },
        decoder,
        parent_task);
}

void ParallelDecoderAdapter::visit(const fmt::BaseAudioDecoder &decoder)
{
    auto input_file = this->input_file;
    parent_task->unpacker.save_file(
        [input_file, &decoder](const Logger &logger)
        {
            io::File file_copy(*input_file);
            return util::file_from_audio(
                decoder.decode(logger, file_copy), input_file->path);
        },
        decoder,
        parent_task);
}
