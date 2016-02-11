#pragma once

#include <string>
#include "io/file_stream.h"

namespace au {
namespace io {

    class File final
    {
    public:
        File(File &other_file);
        File(const io::path &path, std::unique_ptr<io::BaseByteStream> stream);
        File(const io::path &path, const io::FileMode mode);
        File(const io::path &path, const bstr &data);
        File();
        ~File();

        void guess_extension();

    private:
        std::unique_ptr<io::BaseByteStream> stream_holder;
    public:
        io::BaseByteStream &stream;
        io::path path; // doesn't need to be physical path

    };

} }
