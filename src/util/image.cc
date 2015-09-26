#include "util/image.h"
#include <png.h>
#include <jpeglib.h>
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

static void png_read_data(
    png_structp png_ptr, png_bytep data, png_size_t size)
{
    io::IO *io = reinterpret_cast<io::IO*>(png_get_io_ptr(png_ptr));
    io->read(data, size);
}

static void png_flush(png_structp)
{
}

struct Image::Priv final
{
    Priv();
    ~Priv();

    static std::unique_ptr<Image> from_png(io::IO &io);
    static std::unique_ptr<Image> from_jpeg(io::IO &io);
    void save_png(io::IO &io);

    std::unique_ptr<pix::Grid> pixels;
};

Image::Priv::Priv()
{
}

Image::Priv::~Priv()
{
}

std::unique_ptr<Image> Image::Priv::from_png(io::IO &io)
{
    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        throw std::logic_error("Failed to create PNG read structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw std::logic_error("Failed to create PNG info structure");

    png_set_read_fn(png_ptr, &io, &png_read_data);
    png_read_png(
        png_ptr,
        info_ptr,
        PNG_TRANSFORM_GRAY_TO_RGB
            | PNG_TRANSFORM_STRIP_16
            | PNG_TRANSFORM_PACKING
            | PNG_TRANSFORM_BGR
            | PNG_TRANSFORM_EXPAND,
        nullptr);

    int color_type;
    int bits_per_channel;
    png_uint_32 width, height;
    png_get_IHDR(
        png_ptr, info_ptr,
        &width, &height,
        &bits_per_channel, &color_type,
        nullptr, nullptr, nullptr);
    if (bits_per_channel != 8)
        throw err::UnsupportedBitDepthError(bits_per_channel);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    pix::Format format;
    if (color_type == PNG_COLOR_TYPE_RGB)
        format = pix::Format::BGR888;
    else if (color_type == PNG_COLOR_TYPE_RGBA)
        format = pix::Format::BGRA8888;
    else if (color_type == PNG_COLOR_TYPE_GRAY)
        format = pix::Format::Gray8;
    else
        throw err::NotSupportedError("Bad pixel format");

    bstr data;
    data.reserve(width * height * format_to_bpp(format));
    for (auto y : range(height))
        data += bstr(row_pointers[y], width * format_to_bpp(format));
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    std::unique_ptr<Image> image(new Image);
    image->p->pixels.reset(new pix::Grid(width, height, data, format));
    return image;
}

std::unique_ptr<Image> Image::Priv::from_jpeg(io::IO &io)
{
    bstr source = io.read_to_eof();

    jpeg_decompress_struct info;
    jpeg_error_mgr err;
    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);
    jpeg_mem_src(&info, source.get<u8>(), source.size());
    jpeg_read_header(&info, true);
    jpeg_start_decompress(&info);

    auto width = info.output_width;
    auto height = info.output_height;
    auto channels = info.num_components;

    pix::Format format;
    if (channels == 3)
        format = pix::Format::RGB888;
    else if (channels == 4)
        format = pix::Format::RGBA8888;
    else
        throw err::NotSupportedError("Bad pixel format");

    bstr raw_data(width * height * channels);
    for (auto y : range(height))
    {
        auto ptr = raw_data.get<u8>() + y * width * channels;
        jpeg_read_scanlines(&info, &ptr, 1);
    }
    jpeg_finish_decompress(&info);

    std::unique_ptr<Image> image(new Image);
    image->p->pixels.reset(new pix::Grid(width, height, raw_data, format));
    return image;
}

void Image::Priv::save_png(io::IO &io)
{
    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        throw std::logic_error("Failed to create PNG write structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw std::logic_error("Failed to create PNG info structure");

    auto width = pixels->width();
    auto height = pixels->height();
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
    png_set_compression_level(png_ptr, 1);

    png_set_write_fn(png_ptr, &io, &png_write_data, &png_flush);
    png_write_info(png_ptr, info_ptr);

    std::unique_ptr<png_bytep[]> rows(new png_bytep[height]);
    for (auto y : range(height))
        rows.get()[y] = reinterpret_cast<png_bytep>(&pixels->at(0, y));
    png_set_rows(png_ptr, info_ptr, rows.get());
    png_write_png(png_ptr, info_ptr, transformations, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}

Image::Image() : p(new Priv())
{
}

Image::~Image()
{
}

std::unique_ptr<Image> Image::from_pixels(const pix::Grid &pixels)
{
    std::unique_ptr<Image> image(new Image);
    image->p->pixels.reset(new pix::Grid(pixels));
    return image;
}

std::unique_ptr<Image> Image::from_boxed(const bstr &data)
{
    io::BufferedIO tmp_io(data);
    return from_boxed(tmp_io);
}

std::unique_ptr<Image> Image::from_boxed(io::IO &io)
{
    static const bstr png_magic = "\x89PNG"_b;
    static const bstr jpeg_magic = "\xFF\xD8\xFF"_b;

    io.seek(0);
    if (io.read(png_magic.size()) == png_magic)
    {
        io.seek(0);
        return Priv::from_png(io);
    }

    io.seek(0);
    if (io.read(jpeg_magic.size()) == jpeg_magic)
    {
        io.seek(0);
        return Priv::from_jpeg(io);
    }

    throw err::RecognitionError("Not a PNG nor a JPEG file");
}

std::unique_ptr<File> Image::create_file(const std::string &name) const
{
    std::unique_ptr<File> output_file(new File);
    output_file->name = name;
    output_file->change_extension("png");
    p->save_png(output_file->io);
    return output_file;
}

pix::Grid &Image::pixels()
{
    return *p->pixels;
}

const pix::Grid &Image::pixels() const
{
    return *p->pixels;
}
