#pragma once

#include <string>

namespace au {
namespace io {

    class path final
    {
    public:
        path();
        path(const char *s);
        path(const std::string &s);

        const char *c_str() const;
        std::string str() const;
        std::wstring wstr() const;

        path parent() const;
        std::string name() const;
        std::string stem() const;
        std::string extension() const;

        bool is_absolute() const;
        bool is_root() const;
        bool has_extension() const;
        bool has_extension(const std::string &extension) const;
        io::path &change_extension(const std::string &new_extension);
        io::path &change_stem(const std::string &new_extension);

        bool operator ==(const path &other) const;
        bool operator <(const path &other) const;

        path operator /(const path &other) const;
        void operator /=(const path &other);

    private:
        std::string p;
    };

} }
