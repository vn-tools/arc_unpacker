require_relative 'sgd_decompressor'
require_relative '../binary_io'
require_relative '../warning_silencer'
silence_warnings { require 'rmagick' }

# Converts MGD to PNG and vice versa.
class MgdConverter
  MAGIC = 'MGD '

  NO_COMPRESSION = 0
  SGD_COMPRESSION = 1
  PNG_COMPRESSION = 2

  def self.decode(data)
    Reader.new.read(data)
  end

  def self.encode(data, regions = nil)
    Writer.new.write(data, regions)
  end

  class Reader
    def read(data)
      input = BinaryIO.from_string(data)

      magic = input.read(MAGIC.length)
      fail 'Not a MGD picture' if magic != MAGIC

      _data_offset,
      _format,
      image_width,
      image_height,
      _size_original,
      _size_compressed, # == size_compressed + 4
      compression_type, = input.read(24).unpack('SS x4 S2 L2 L')

      _unknown = input.read(64)
      size_compressed, = input.read(4).unpack('L')

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
        data = input.read(size_compressed)
        uncompressed_to_png(data, image_width, image_height)

      else
        fail 'Unsupported compression type.'
      end

      regions = read_regions(input)
      [png_data, regions]
    end

    def read_regions(input)
      fail 'Failed to read regions metadata' if input.eof?
      regions = []
      unless input.eof?
        regions_size,
        region_count,
        meta_format = input.read(12).unpack('x4 LSS')
        bytes_left = input.length - input.tell

        fail format('Unsupported format (%d)',  meta_format) if meta_format != 4
        fail 'Failed to read regions metadata' if regions_size != bytes_left

        region_count.times do
          dim = input.read(8).unpack('S*')
          regions.push(
            width: dim[2],
            height: dim[3],
            x: dim[0],
            y: dim[1])
        end

        _unknown = input.read(4)
      end

      regions
    end

    def uncompressed_to_png(blob, image_width, image_height)
      image = Magick::Image.new(image_width, image_height)
      image.import_pixels(0, 0, image_width, image_height, 'BGRA', blob)
      image.to_blob { self.format = 'PNG' }
    end

    def sgd_to_png(input, image_width, image_height)
      output = SgdDecompressor.decompress(input)
      uncompressed_to_png(output, image_width, image_height)
    end
  end

  class Writer
    def write(data, regions = nil)
      image = Magick::Image.from_blob(data)[0]
      blob = image.export_pixels_to_str(0, 0, image.columns, image.rows, 'BGRA')
      blob_size = blob.length

      data_offset = 92
      output = BinaryIO.from_string('')
      output.write(MAGIC)
      output.write([
        data_offset,
        6,
        image.columns,
        image.rows,
        image.columns * image.rows * 4,
        blob_size + 4,
        NO_COMPRESSION,
        blob_size].pack('SS x4 S2 L2 L x64 L'))
      output.write(blob)

      write_regions(output, regions, image.columns, image.rows)

      output.rewind
      output.read
    end

    def write_regions(output, regions, image_width, image_height)
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
  end
end
