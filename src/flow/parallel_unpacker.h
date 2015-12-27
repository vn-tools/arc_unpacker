#pragma once

#include <map>
#include <memory>
#include "file_saver.h"
#include "fmt/registry.h"
#include "logger.h"
#include "task_scheduler.h"

namespace au {
namespace flow {

    class ParallelUnpacker;

    using FileFactory = std::function<std::shared_ptr<io::File>()>;
    using FileFactoryWithLogger
        = std::function<std::shared_ptr<io::File>(const Logger &)>;

    struct ParallelUnpackerContext final
    {
        ParallelUnpackerContext(
            const Logger &logger,
            const FileSaver &file_saver,
            const fmt::Registry &registry,
            const bool enable_nested_decoding,
            const std::vector<std::string> &arguments,
            const std::vector<std::string> &available_decoders);

        const Logger &logger;
        const FileSaver &file_saver;
        const fmt::Registry &registry;
        const bool enable_nested_decoding;
        const std::vector<std::string> arguments;
        const std::vector<std::string> available_decoders;
    };

    struct BaseParallelUnpackingTask : public ITask
    {
        BaseParallelUnpackingTask(
            ParallelUnpacker &unpacker,
            TaskScheduler &task_scheduler,
            const ParallelUnpackerContext &unpacker_context,
            const size_t depth,
            const io::path &base_name);

        Logger logger;
        ParallelUnpacker &unpacker;
        TaskScheduler &task_scheduler;
        const ParallelUnpackerContext &unpacker_context;
        const size_t depth;
        const io::path base_name;
        unsigned int task_id;
    };

    class ParallelUnpacker final
    {
    public:
        ParallelUnpacker(const ParallelUnpackerContext &context);
        ~ParallelUnpacker();

        void add_input_file(const io::path &base_name, const FileFactory);

        void save_file(
            const FileFactoryWithLogger,
            const std::shared_ptr<const fmt::IDecoder> origin_decoder,
            const io::path &origin_path,
            const BaseParallelUnpackingTask &origin_task);

        bool run(const size_t thread_count = 0);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
