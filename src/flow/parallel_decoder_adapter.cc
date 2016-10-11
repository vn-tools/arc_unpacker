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
            decoder,
            entry->path.str());
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
