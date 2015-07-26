#ifndef AU_FILE_H
#define AU_FILE_H
#include <boost/filesystem/path.hpp>
#include <string>
#include "io/file_io.h"

namespace au {

    class File final
    {
    public:
        File(const boost::filesystem::path &path, const io::FileMode mode);
        File();
        ~File();
        io::IO &io;
        std::string name;
        bool has_extension(const std::string &extension);
        void change_extension(const std::string &new_extension);
        void guess_extension();
    };

}

#endif
