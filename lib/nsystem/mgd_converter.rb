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
    _size_compressed, # == size_compressed + 4
    compression_type,
    size_compressed = input.read(92).unpack('SS x4 S2 L2 L x64 L')

    png_data =
    case compression_type
    when PNG_COMPRESSION
      size_compressed = input.read.index('IEND') + 8
      input.seek(96)
      input.read(size_compressed)

    when SGD_COMPRESSION
      data = input.read(size_compressed)
      sgd_to_png(data, image_width, image_height)

    when NO_COMPRESSION
      fail 'Unsupported compression type.'

    else
      fail 'Unsupported compression type.'
    end

    regions = read_regions(input)
    [png_data, regions]
  end

  def self.read_regions(input)
    fail 'Failed to read regions metadata' if input.eof?
    regions = []
    unless input.eof?
      regions_size,
      region_count,
      meta_format = input.read(12).unpack('x4 LSS')
      bytes_left = input.length - input.tell

      fail 'Unsupported format' if meta_format != 4
      fail 'Failed to read regions metadata' if regions_size != bytes_left

      region_count.times do
        dim = input.read(8).unpack('S*')
        regions.push(
          width: dim[2],
          height: dim[3],
          x: dim[0],
          y: dim[1])
      end
    end

    regions
  end

  def self.encode(data, regions = nil)
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
      png_size].pack('SS x4 S2 L2 L x64 L'))
    output.write(png_blob)

    write_regions(output, regions, image.columns, image.rows)

    output.rewind
    output.read
  end

  def self.write_regions(output, regions, image_width, image_height)
    regions = [
      width: image_width,
      height: image_height,
      x: 0,
      y: 0
    ] if regions.nil?

    output.write([
      regions.count * 8 + 4,
      regions.count,
      4].pack('x4 L S2'))

    regions.each do |si|
      output.write([si[:x], si[:y], si[:width], si[:height]].pack('S4'))
    end

    output.write("\x00" * 4)
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
    output = SgdDecompressor.decompress(input)
    uncompressed_to_png(output, image_width, image_height)
  end
end
