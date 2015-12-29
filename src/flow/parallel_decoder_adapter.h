#pragma once

#include "flow/parallel_unpacker.h"
#include "fmt/idecoder_visitor.h"

namespace au {
namespace flow {

    class ParallelDecoderAdapter final : public fmt::IDecoderVisitor
    {
    public:
        ParallelDecoderAdapter(
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
            const std::shared_ptr<io::File> input_file);
        ~ParallelDecoderAdapter();

        void visit(const fmt::BaseArchiveDecoder &decoder) override;
        void visit(const fmt::BaseFileDecoder &decoder) override;
        void visit(const fmt::BaseImageDecoder &decoder) override;
        void visit(const fmt::BaseAudioDecoder &decoder) override;

    private:
        const std::shared_ptr<const BaseParallelUnpackingTask> parent_task;
        const std::shared_ptr<io::File> input_file;
    };

} }
