#pragma once

#include <string>
#include "io/file_stream.h"

namespace au {
namespace io {

    class File final
    {
    public:
        File(File &other_file);
        File(const io::path &path, const io::FileMode mode);
        File(const io::path &path, const bstr &data);
        File();
        ~File();

        void guess_extension();

    private:
        std::unique_ptr<io::IStream> stream_holder;
    public:
        io::IStream &stream;
        io::path path; // doesn't need to be physical path

    };

} }
