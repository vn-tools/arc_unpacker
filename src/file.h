#pragma once

#include <boost/filesystem/path.hpp>
#include <string>
#include "io/file_io.h"

namespace au {

    class File final
    {
    public:
        File(const boost::filesystem::path &path, const io::FileMode mode);
        File(const std::string &name, const bstr &data);
        File();
        ~File();
        bool has_extension();
        bool has_extension(const std::string &extension);
        void change_extension(const std::string &new_extension);
        void guess_extension();

        io::IO &io;
        std::string name;
    };

}
