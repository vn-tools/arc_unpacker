#pragma once

#include <map>
#include <memory>
#include "file_saver.h"
#include "flow/task_scheduler.h"
#include "fmt/base_decoder.h"
#include "fmt/registry.h"
#include "logger.h"

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
            const io::path &base_name,
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task);

        virtual ~BaseParallelUnpackingTask() {}

        Logger logger;
        ParallelUnpacker &unpacker;
        TaskScheduler &task_scheduler;
        const ParallelUnpackerContext &unpacker_context;
        const size_t depth;
        const io::path base_name;
        unsigned int task_id;
        const std::shared_ptr<const BaseParallelUnpackingTask> parent_task;
    };

    class ParallelUnpacker final
    {
    public:
        ParallelUnpacker(const ParallelUnpackerContext &context);
        ~ParallelUnpacker();

        void add_input_file(const io::path &base_name, const FileFactory);

        void save_file(
            const FileFactoryWithLogger,
            const fmt::BaseDecoder &origin_decoder,
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task);

        bool run(const size_t thread_count = 0);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
