#pragma once

#include <memory>
#include "file.h"
#include "io/io.h"
#include "pix/grid.h"

namespace au {
namespace util {

    std::unique_ptr<File> grid_to_boxed(
        const pix::Grid &pixels, const std::string &name);

    class Image final
    {
    public:
        ~Image();

        static std::unique_ptr<Image> from_boxed(const bstr &data);
        static std::unique_ptr<Image> from_boxed(io::IO &io);
        static std::unique_ptr<Image> from_pixels(const pix::Grid &pixels);

        pix::Grid &pixels();
        const pix::Grid &pixels() const;

    private:
        Image();

        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
