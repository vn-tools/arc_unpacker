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

#pragma once

#include <map>
#include <memory>
#include <set>
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
            const std::set<std::string> &decoders_to_check);

        const Logger &logger;
        const IFileSaver &file_saver;
        const dec::Registry &registry;
        const bool enable_nested_decoding;
        const std::vector<std::string> arguments;
        const std::set<std::string> decoders_to_check;
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
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
            const std::set<std::string> &decoders_to_check);

        virtual ~BaseParallelUnpackingTask() {}

        size_t get_depth() const;

        void save_file(
            const std::shared_ptr<io::File> input_file,
            const DecoderFileFactory,
            const dec::BaseDecoder &origin_decoder,
            const std::string &custom_name = "") const;

        Logger logger;
        ParallelTaskContext &task_context;
        const TaskSourceType source_type;
        const io::path base_name;
        const std::shared_ptr<const BaseParallelUnpackingTask> parent_task;
        const std::set<std::string> decoders_to_check;
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
