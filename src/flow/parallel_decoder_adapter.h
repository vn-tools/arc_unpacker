#pragma once

#include "fmt/idecoder_visitor.h"
#include "parallel_unpacker.h"

namespace au {
namespace flow {

    class ParallelDecoderAdapter final : public fmt::IDecoderVisitor
    {
    public:
        ParallelDecoderAdapter(
            const BaseParallelUnpackingTask &task,
            const std::shared_ptr<io::File> input_file);
        ~ParallelDecoderAdapter();

        void visit(const fmt::BaseArchiveDecoder &decoder) override;
        void visit(const fmt::BaseFileDecoder &decoder) override;
        void visit(const fmt::BaseImageDecoder &decoder) override;
        void visit(const fmt::BaseAudioDecoder &decoder) override;

    private:
        const BaseParallelUnpackingTask &task;
        const std::shared_ptr<io::File> input_file;
    };

} }
