#pragma once

#include <boost/filesystem.hpp>
#include "io/path.h"

namespace au {
namespace io {

    path current_working_directory();
    bool exists(const path &p);
    bool is_directory(const path &p);
    bool is_regular_file(const path &p);
    path absolute(const path &p);

    void create_directories(const path &p);
    void remove(const path &p);

    template<typename T> class BaseDirectoryRange final
    {
    public:
        struct Iterator final
            : std::iterator<std::random_access_iterator_tag, path, path>
        {
            T it;

            inline Iterator(const T it) : it(it)
            {
            }

            inline path operator *()
            {
                return path(it->path().string());
            }

            inline Iterator operator ++()
            {
                it++;
                return *this;
            }

            inline bool operator !=(Iterator other)
            {
                return it != other.it;
            }
        };

        inline BaseDirectoryRange(const path &path) :
            path_copy(path),
            b(Iterator(T(path_copy.str()))),
            e(Iterator(T()))
        {
        }

        inline Iterator begin()
        {
            return b;
        }

        inline Iterator end()
        {
            return e;
        }

    private:
        path path_copy;
        Iterator b, e;
    };

    using DirectoryRange = BaseDirectoryRange
        <boost::filesystem::directory_iterator>;

    using RecursiveDirectoryRange = BaseDirectoryRange
        <boost::filesystem::recursive_directory_iterator>;

    inline DirectoryRange directory_range(const path &path)
    {
        return DirectoryRange(path);
    }

    inline RecursiveDirectoryRange recursive_directory_range(const path &path)
    {
        return RecursiveDirectoryRange(path);
    }

} }
