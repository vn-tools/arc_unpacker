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

#include "base_decoder.h"

namespace au {
namespace dec {

    struct ArchiveEntry
    {
        virtual ~ArchiveEntry() {}
        io::path path;
    };

    struct PlainArchiveEntry : ArchiveEntry
    {
        virtual ~PlainArchiveEntry() {}
        uoff_t offset;
        size_t size;
    };

    struct CompressedArchiveEntry : ArchiveEntry
    {
        virtual ~CompressedArchiveEntry() {}
        uoff_t offset;
        size_t size_orig, size_comp;
    };

    struct ArchiveMeta
    {
        virtual ~ArchiveMeta() {}
        std::vector<std::unique_ptr<ArchiveEntry>> entries;
    };

    class BaseArchiveDecoder : public BaseDecoder
    {
    public:
        BaseArchiveDecoder();

        virtual ~BaseArchiveDecoder() {}

        virtual algo::NamingStrategy naming_strategy() const override;

        void accept(IDecoderVisitor &visitor) const override;

        std::unique_ptr<ArchiveMeta> read_meta(
            const Logger &logger, io::File &input_file) const;

        std::unique_ptr<io::File> read_file(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const;

    protected:
        virtual std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const = 0;

        virtual std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const = 0;

    private:
        bool numeric_file_names;
    };

} }
