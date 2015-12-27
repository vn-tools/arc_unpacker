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

void ParallelDecoderAdapter::do_save(
    const fmt::BaseDecoder &decoder,
    const std::function<std::shared_ptr<io::File>()> file_factory) const
{
    task.unpacker.save_file(
        file_factory,
        decoder.shared_from_this(),
        input_file->path,
        task);
}

void ParallelDecoderAdapter::visit(const fmt::ArchiveDecoder &decoder)
{
    auto input_file = this->input_file;
    auto meta = std::shared_ptr<fmt::ArchiveMeta>(
        decoder.read_meta(task.logger, *input_file));
    task.logger.info(
        "%s: archive contains %d files.\n",
        input_file->path.c_str(),
        meta->entries.size());
    auto decoder_copy = decoder.shared_from_this();
    auto logger_copy = task.logger;
    for (const auto &entry : meta->entries)
    {
        util::VirtualFileSystem::register_file(
            fmt::decorate_path(
                decoder.naming_strategy(), task.base_name, entry->path),
            [input_file, meta, &entry, logger_copy, decoder_copy]()
            {
                io::File file_copy(*input_file);
                return static_cast<const fmt::ArchiveDecoder&>(*decoder_copy)
                    .read_file(logger_copy, file_copy, *meta, *entry);
            });
    }

    for (const auto &entry : meta->entries)
    {
        do_save(
            decoder,
            [input_file, meta, &entry, logger_copy, &decoder]()
            {
                io::File file_copy(*input_file);
                return decoder.read_file(logger_copy, file_copy, *meta, *entry);
            });
    }
}

void ParallelDecoderAdapter::visit(const fmt::FileDecoder &decoder)
{
    auto input_file = this->input_file;
    auto logger_copy = task.logger;
    do_save(
        decoder,
        [input_file, logger_copy, &decoder]()
        {
            io::File file_copy(*input_file);
            return decoder.decode(logger_copy, file_copy);
        });
}

void ParallelDecoderAdapter::visit(const fmt::ImageDecoder &decoder)
{
    auto input_file = this->input_file;
    auto logger_copy = task.logger;
    do_save(
        decoder,
        [input_file, logger_copy, &decoder]()
        {
            io::File file_copy(*input_file);
            return util::file_from_image(
                decoder.decode(logger_copy, file_copy), input_file->path);
        });
}

void ParallelDecoderAdapter::visit(const fmt::AudioDecoder &decoder)
{
    auto input_file = this->input_file;
    auto logger_copy = task.logger;
    do_save(
        decoder,
        [input_file, logger_copy, &decoder]()
        {
            io::File file_copy(*input_file);
            return util::file_from_audio(
                decoder.decode(logger_copy, file_copy), input_file->path);
        });
}
