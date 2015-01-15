require 'lib/binary_io'
require 'lib/image'
require_relative 'mgd_converter/sgd_compressor'

# Converts MGD to PNG and vice versa.
# Seen in FJSYS archives.
module MgdConverter
  module_function

  MAGIC = 'MGD '
  NO_COMPRESSION = 0
  SGD_COMPRESSION = 1
  PNG_COMPRESSION = 2

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    image = Decoder.new.read(file.data)
    image.update_file(file)
  end

  def encode!(file, _options)
    file.data = Encoder.new.write(file.data)
    file.change_extension('.mgd')
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)

      magic = input.read(MAGIC.length)
      fail RecognitionError, 'Not a MGD picture' if magic != MAGIC

      _data_offset,
      _format,
      width,
      height,
      _size_original,
      _size_compressed, # == size_compressed + 4
      compression_type, = input.read(24).unpack('SS x4 S2 L2 L')

      _unknown = input.read(64)
      size_compressed, = input.read(4).unpack('L')

      case compression_type
      when PNG_COMPRESSION
        size_compressed = input.read.index('IEND') + 8
        input.seek(96)
        raw_data = Image.from_boxed(input.read(size_compressed), 'BGRA').pixels

      when SGD_COMPRESSION
        raw_data = SgdCompressor.decode(input.read(size_compressed))

      when NO_COMPRESSION
        raw_data = input.read(size_compressed)

      else
        fail 'Unsupported compression type.'
      end

      regions = read_regions(input)
      Image.from_pixels(width, height, raw_data, 'BGRA', regions: regions)
    end

    def read_regions(input)
      fail RecognitionError, 'Failed to read regions metadata' if input.eof?
      regions = []
      unless input.eof?
        regions_size,
        region_count,
        meta_format = input.read(12).unpack('x4 LSS')
        bytes_left = input.length - input.tell

        fail format('Unsupported format (%d)',  meta_format) if meta_format != 4
        if regions_size != bytes_left
          fail RecognitionError, 'Failed to read regions metadata'
        end

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
  end

  class Encoder
    def write(data)
      image = Image.from_boxed(data, 'BGRA')

      data_offset = 92
      output = BinaryIO.from_string('')
      output.write(MAGIC)
      output.write([
        data_offset,
        6,
        image.width,
        image.height,
        image.width * image.height * 4,
        image.pixels.length + 4,
        NO_COMPRESSION,
        image.pixels.length].pack('SS x4 S2 L2 L x64 L'))

      output.write(image.pixels)

      write_regions(output, image.meta[:regions], image.width, image.height)

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
