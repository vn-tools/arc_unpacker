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
