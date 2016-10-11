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

#include <memory>
#include <string>
#include "io/base_byte_stream.h"
#include "io/path.h"

namespace au {
namespace io {

    enum class FileMode : u8
    {
        Read = 1,
        Write = 2,
    };

    class FileByteStream final : public BaseByteStream
    {
    public:
        FileByteStream(const path &path, const FileMode mode);
        ~FileByteStream();

        uoff_t size() const override;
        uoff_t pos() const override;

        std::unique_ptr<BaseByteStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;
        void seek_impl(const uoff_t offset) override;
        void resize_impl(const uoff_t new_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
