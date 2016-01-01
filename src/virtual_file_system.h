#pragma once

#include <functional>
#include <memory>
#include "io/file.h"
#include "io/path.h"

namespace au {

    class VirtualFileSystem final
    {
    public:
        static void enable();
        static void disable();

        static void register_file(
            const io::path &path,
            const std::function<std::unique_ptr<io::File>()> factory);
        static void unregister_file(const io::path &path);

        static void register_directory(const io::path &path);
        static void unregister_directory(const io::path &path);

        static std::unique_ptr<io::File> get_by_stem(const std::string &stem);
        static std::unique_ptr<io::File> get_by_name(const std::string &name);
        static std::unique_ptr<io::File> get_by_path(const io::path &path);
    };

}
