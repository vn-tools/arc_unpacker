#pragma once

#include <map>
#include <memory>
#include "dec/base_decoder.h"
#include "dec/registry.h"
#include "flow/ifile_saver.h"
#include "flow/task_scheduler.h"
#include "logger.h"

namespace au {
namespace flow {

    enum class TaskSourceType : u8
    {
        InitialUserInput,
        NestedDecoding,
    };

    class ParallelUnpacker;

    using InputFileFactory = std::function<std::shared_ptr<io::File>()>;
    using DecoderFileFactory
        = std::function<std::shared_ptr<io::File>(io::File &, const Logger &)>;

    struct ParallelUnpackerContext final
    {
        ParallelUnpackerContext(
            const Logger &logger,
            const IFileSaver &file_saver,
            const dec::Registry &registry,
            const bool enable_nested_decoding,
            const std::vector<std::string> &arguments,
            const std::vector<std::string> &available_decoders);

        const Logger &logger;
        const IFileSaver &file_saver;
        const dec::Registry &registry;
        const bool enable_nested_decoding;
        const std::vector<std::string> arguments;
        const std::vector<std::string> available_decoders;
    };

    struct ParallelTaskContext final
    {
        ParallelTaskContext(
            ParallelUnpacker &unpacker,
            const ParallelUnpackerContext &unpacker_context,
            TaskScheduler &task_scheduler);

        ParallelUnpacker &unpacker;
        const ParallelUnpackerContext &unpacker_context;
        TaskScheduler &task_scheduler;
    };

    struct BaseParallelUnpackingTask :
        public ITask,
        public std::enable_shared_from_this<BaseParallelUnpackingTask>
    {
        BaseParallelUnpackingTask(
            ParallelTaskContext &task_context,
            const TaskSourceType source_type,
            const io::path &base_name,
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task);

        virtual ~BaseParallelUnpackingTask() {}

        size_t get_depth() const;

        void save_file(
            const std::shared_ptr<io::File> input_file,
            const DecoderFileFactory,
            const dec::BaseDecoder &origin_decoder) const;

        Logger logger;
        ParallelTaskContext &task_context;
        const TaskSourceType source_type;
        const io::path base_name;
        const std::shared_ptr<const BaseParallelUnpackingTask> parent_task;
    };

    class ParallelUnpacker final
    {
    public:
        ParallelUnpacker(const ParallelUnpackerContext &unpacker_context);
        ~ParallelUnpacker();

        void add_input_file(const io::path &base_name, const InputFileFactory);
        bool run(const size_t thread_count = 0);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
