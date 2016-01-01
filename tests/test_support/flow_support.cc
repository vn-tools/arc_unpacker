#include "test_support/flow_support.h"
#include "flow/file_saver_callback.h"
#include "flow/parallel_unpacker.h"

using namespace au;

std::vector<std::shared_ptr<io::File>> tests::flow_unpack(
    const dec::Registry &registry,
    const bool enable_nested_decoding,
    io::File &input_file)
{
    Logger dummy_logger;
    dummy_logger.mute();

    std::vector<std::shared_ptr<io::File>> saved_files;
    const flow::FileSaverCallback file_saver(
        [&](std::shared_ptr<io::File> saved_file)
        {
            saved_file->stream.seek(0);
            saved_files.push_back(saved_file);
        });

    flow::ParallelUnpackerContext context(
        dummy_logger,
        file_saver,
        registry,
        enable_nested_decoding,
        {},
        registry.get_decoder_names());

    flow::ParallelUnpacker unpacker(context);
    unpacker.add_input_file(
        input_file.path,
        [&]()
        {
            return std::make_shared<io::File>(input_file);
        });
    unpacker.run(1);
    return saved_files;
}
