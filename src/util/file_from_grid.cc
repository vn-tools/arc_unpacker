#include "util/file_from_grid.h"
#include <png.h>
#include "err.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::util;

static void png_write_data(
    png_structp png_ptr, png_bytep data, png_size_t size)
{
    io::IO *io = reinterpret_cast<io::IO*>(png_get_io_ptr(png_ptr));
    io->write(data, size);
}

static void png_flush(png_structp)
{
}

std::unique_ptr<File> util::file_from_grid(
    const pix::Grid &pixels, const std::string &name)
{
    auto output_file = std::make_unique<File>();
    output_file->name = name;
    output_file->change_extension("png");

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        throw std::logic_error("Failed to create PNG write structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw std::logic_error("Failed to create PNG info structure");

    auto width = pixels.width();
    auto height = pixels.height();
    if (!width || !height)
        throw err::BadDataSizeError();
    const auto bpp = 4;
    const auto color_type = PNG_COLOR_TYPE_RGBA;
    int transformations = PNG_TRANSFORM_BGR;

    png_set_IHDR(
        png_ptr, info_ptr, width, height, 8, color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    // 0 = no compression, 9 = max compression
    // 1 produces good file size and is still fast.
    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    png_set_compression_level(png_ptr, 1);

    png_set_write_fn(png_ptr, &output_file->io, &png_write_data, &png_flush);
    png_write_info(png_ptr, info_ptr);

    auto rows = std::make_unique<const u8*[]>(height);
    for (auto y : range(height))
        rows.get()[y] = reinterpret_cast<const u8*>(&pixels.at(0, y));
    png_set_rows(png_ptr, info_ptr, const_cast<u8**>(rows.get()));
    png_write_png(png_ptr, info_ptr, transformations, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return output_file;
}
