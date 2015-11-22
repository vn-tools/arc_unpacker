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

    class directory_range final
    {
    public:
        struct Iterator final
            : std::iterator<std::random_access_iterator_tag, path, path>
        {
            boost::filesystem::directory_iterator it;

            Iterator(
                boost::filesystem::directory_iterator it) : it(it)
            {
            }

            path operator *()
            {
                return path(it->path().string());
            }

            Iterator operator ++()
            {
                it++;
                return *this;
            }

            bool operator !=(Iterator other)
            {
                return it != other.it;
            }
        };

        directory_range(const path &path) :
            path_copy(path),
            b(Iterator(boost::filesystem::directory_iterator(path_copy.str()))),
            e(Iterator(boost::filesystem::directory_iterator()))
        {
        }

        Iterator begin()
        {
            return b;
        }

        Iterator end()
        {
            return e;
        }

    private:
        path path_copy;
        Iterator b, e;
    };

    class recursive_directory_range final
    {
    public:
        struct Iterator final
            : std::iterator<std::random_access_iterator_tag, path, path>
        {
            boost::filesystem::recursive_directory_iterator it;

            Iterator(
                boost::filesystem::recursive_directory_iterator it) : it(it)
            {
            }

            path operator *()
            {
                return path(it->path().string());
            }

            Iterator operator ++()
            {
                it++;
                return *this;
            }

            bool operator !=(Iterator other)
            {
                return it != other.it;
            }
        };

        recursive_directory_range(const path &path) :
            path_copy(path),
            b(Iterator(boost::filesystem::recursive_directory_iterator(
                path_copy.str()))),
            e(Iterator(boost::filesystem::recursive_directory_iterator()))
        {
        }

        Iterator begin()
        {
            return b;
        }

        Iterator end()
        {
            return e;
        }

    private:
        path path_copy;
        Iterator b, e;
    };

} }
