require 'rmagick'
require_relative '../binary_io'
require_relative 'sgd_decompressor'

# Converts MGD to PNG and vice versa.
class MgdConverter
  MAGIC = 'MGD '

  NO_COMPRESSION = 0
  SGD_COMPRESSION = 1
  PNG_COMPRESSION = 2

  def self.decode(data)
    input = BinaryIO.new(data)

    magic = input.read(MAGIC.length)
    fail 'Not a MGD picture' if magic != MAGIC

    _data_offset,
    _format,
    image_width,
    image_height,
    _size_original,
    _size_compressed,
    compression_type = input.read(88).unpack('SS x4 S2 L2 S x66')

    case compression_type
    when PNG_COMPRESSION
      input.seek(4, IO::SEEK_CUR)
      return input.read
    when NO_COMPRESSION
      return uncompressed_to_png(input.read, image_width, image_height)
    when SGD_COMPRESSION
      return sgd_to_png(input.read, image_width, image_height)
    end

    fail 'Not supported?'
  end

  def self.encode(data)
    image = Magick::Image.from_blob(data)[0]
    png_blob = image.to_blob do
      self.format = 'PNG'
      self.quality = 0
    end
    png_size = png_blob.length

    data_offset = 92
    output = BinaryIO.new
    output.write(MAGIC)
    output.write([
      data_offset,
      1,
      image.columns,
      image.rows,
      image.columns * image.rows * 4,
      png_size + 4,
      PNG_COMPRESSION,
      png_size].pack('SS x4 S2 L2 S x66 L'))
    output.write(png_blob)
    output.rewind
    output.read
  end

  def self.uncompressed_to_png(input, image_width, image_height)
    image = Magick::Image.from_blob(input) do
      self.format = 'RGBA'
      self.depth = 8
      self.size = "#{image_width}x#{image_height}"
    end

    image = image[0]
    image.to_blob do
      self.format = 'PNG'
    end
  end

  def self.sgd_to_png(input, image_width, image_height)
    compressed_length = input[0..3].unpack('L')[0]
    output = SgdDecompressor.decompress(input[4..4 + compressed_length])
    uncompressed_to_png(output, image_width, image_height)
  end
end
