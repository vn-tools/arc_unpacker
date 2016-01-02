#include "flow/parallel_decoder_adapter.h"
#include "algo/naming_strategies.h"
#include "enc/microsoft/wav_audio_encoder.h"
#include "enc/png/png_image_encoder.h"
#include "flow/vfs_bridge.h"

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

void ParallelDecoderAdapter::visit(const dec::BaseArchiveDecoder &decoder)
{
    auto input_file = this->input_file;
    auto meta = std::shared_ptr<dec::ArchiveMeta>(
        decoder.read_meta(parent_task->logger, *input_file));
    parent_task->logger.info(
        "archive contains %d files.\n", meta->entries.size());

    const auto vfs_bridge = std::make_shared<VirtualFileSystemBridge>(
        parent_task->logger,
        decoder,
        meta,
        input_file,
        parent_task->base_name);

    for (const auto &entry : meta->entries)
    {
        parent_task->save_file(
            input_file,
            [meta, &entry, &decoder, vfs_bridge]
            (io::File &input_file_copy, const Logger &logger)
            {
                return decoder.read_file(
                    logger, input_file_copy, *meta, *entry);
            },
            decoder);
    }
}

void ParallelDecoderAdapter::visit(const dec::BaseFileDecoder &decoder)
{
    parent_task->save_file(
        input_file,
        [&decoder](io::File &input_file_copy, const Logger &logger)
        {
            return decoder.decode(logger, input_file_copy);
        },
        decoder);
}

void ParallelDecoderAdapter::visit(const dec::BaseImageDecoder &decoder)
{
    parent_task->save_file(
        input_file,
        [&decoder](io::File &input_file_copy, const Logger &logger)
        {
            auto output_file = decoder.decode(logger, input_file_copy);
            const auto encoder = enc::png::PngImageEncoder();
            return encoder.encode(logger, output_file, input_file_copy.path);
        },
        decoder);
}

void ParallelDecoderAdapter::visit(const dec::BaseAudioDecoder &decoder)
{
    parent_task->save_file(
        input_file,
        [&decoder](io::File &input_file_copy, const Logger &logger)
        {
            auto output_file = decoder.decode(logger, input_file_copy);
            const auto encoder = enc::microsoft::WavAudioEncoder();
            return encoder.encode(logger, output_file, input_file_copy.path);
        },
        decoder);
}
