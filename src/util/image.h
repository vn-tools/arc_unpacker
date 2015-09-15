#pragma once

#include <memory>
#include <string>
#include "file.h"
#include "io/io.h"
#include "pix/grid.h"

namespace au {
namespace util {

    class Image final
    {
    public:
        ~Image();

        static std::unique_ptr<Image> from_boxed(const bstr &data);
        static std::unique_ptr<Image> from_boxed(io::IO &io);
        static std::unique_ptr<Image> from_pixels(const pix::Grid &pixels);

        std::unique_ptr<File> create_file(const std::string &name) const;

        pix::Grid &pixels();
        const pix::Grid &pixels() const;

    private:
        Image(size_t width, size_t height);

        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
