#include "enc/microsoft/bmp_image_encoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::enc::microsoft;

void BmpImageEncoder::encode_impl(
    const Logger &logger,
    const res::Image &input_image,
    io::File &output_file) const
{
    const auto width = input_image.width();
    const auto height = input_image.height();
    const auto stride = ((width * 3) + 3) & ~3;

    // BITMAPFILEHEADER
    output_file.stream.write("BM"_b);
    output_file.stream.write_le<u32>(14 + 40 + stride * height); // bfSize
    output_file.stream.write_le<u16>(0);        // bfReserved1
    output_file.stream.write_le<u16>(0);        // bfReserved2
    output_file.stream.write_le<u32>(14 + 40);  // bfOffBits

    // BITMAPINFOHEADER
    output_file.stream.write_le<u32>(40);       // biSize
    output_file.stream.write_le<u32>(width);    // biWidth
    output_file.stream.write_le<u32>(-height);  // biHeight
    output_file.stream.write_le<u16>(1);        // biPlanes
    output_file.stream.write_le<u16>(24);       // biBitCount
    output_file.stream.write_le<u32>(0);        // biCompression
    output_file.stream.write_le<u32>(0);        // biSizeImage
    output_file.stream.write_le<u32>(0);        // biXPelsPerMeter
    output_file.stream.write_le<u32>(0);        // biYPelsPerMeter
    output_file.stream.write_le<u32>(0);        // biClrUsed
    output_file.stream.write_le<u32>(0);        // biClrImportant

    for (const auto y : algo::range(height))
    {
        for (const auto x : algo::range(width))
        {
            const auto &c = input_image.at(x, y);
            output_file.stream.write<u8>(c.b);
            output_file.stream.write<u8>(c.g);
            output_file.stream.write<u8>(c.r);
        }
        output_file.stream.write(bstr(stride - 3 * width));
    }

    output_file.path.change_extension("bmp");
}
