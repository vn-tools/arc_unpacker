#pragma once

#include <string>
#include "io/file_stream.h"

namespace au {
namespace io {

    class File final
    {
    public:
        File(const io::path &name, const io::FileMode mode);
        File(const io::path &name, const bstr &data);
        File();
        ~File();

        void guess_extension();

        io::Stream &stream;
        io::path name;
    };

} }
