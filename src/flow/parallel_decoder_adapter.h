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

        void visit(const fmt::ArchiveDecoder &decoder) override;
        void visit(const fmt::FileDecoder &decoder) override;
        void visit(const fmt::ImageDecoder &decoder) override;
        void visit(const fmt::AudioDecoder &decoder) override;

    private:
        void do_save(
            const fmt::BaseDecoder &decoder,
            const std::function<std::shared_ptr<io::File>()>
                file_factory) const;

        const BaseParallelUnpackingTask &task;
        const std::shared_ptr<io::File> input_file;
    };

} }
