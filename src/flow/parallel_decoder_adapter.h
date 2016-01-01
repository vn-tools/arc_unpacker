#pragma once

#include "dec/idecoder_visitor.h"
#include "flow/parallel_unpacker.h"

namespace au {
namespace flow {

    class ParallelDecoderAdapter final : public dec::IDecoderVisitor
    {
    public:
        ParallelDecoderAdapter(
            const std::shared_ptr<const BaseParallelUnpackingTask> parent_task,
            const std::shared_ptr<io::File> input_file);
        ~ParallelDecoderAdapter();

        void visit(const dec::BaseArchiveDecoder &decoder) override;
        void visit(const dec::BaseFileDecoder &decoder) override;
        void visit(const dec::BaseImageDecoder &decoder) override;
        void visit(const dec::BaseAudioDecoder &decoder) override;

    private:
        const std::shared_ptr<const BaseParallelUnpackingTask> parent_task;
        const std::shared_ptr<io::File> input_file;
    };

} }
