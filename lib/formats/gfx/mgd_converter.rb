require 'lib/binary_io'
require 'lib/warning_silencer'
require 'lib/image'
require_relative 'sgd_compressor'

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

  def decode(data, _options)
    Decoder.new.read(data)
  end

  def encode(data, _options)
    Encoder.new.write(data)
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)

      magic = input.read(MAGIC.length)
      fail 'Not a MGD picture' if magic != MAGIC

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
        raw_data = Image.boxed_to_raw(input.read(size_compressed), 'BGRA')[0]

      when SGD_COMPRESSION
        raw_data = SgdCompressor.decode(input.read(size_compressed))

      when NO_COMPRESSION
        raw_data = input.read(size_compressed)

      else
        fail 'Unsupported compression type.'
      end

      regions = read_regions(input)
      Image.raw_to_boxed(width, height, raw_data, 'BGRA', regions: regions)
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
  end

  class Encoder
    def write(data)
      raw_data, meta = Image.boxed_to_raw(data, 'BGRA')

      data_offset = 92
      output = BinaryIO.from_string('')
      output.write(MAGIC)
      output.write([
        data_offset,
        6,
        meta[:width],
        meta[:height],
        meta[:width] * meta[:height] * 4,
        raw_data.length + 4,
        NO_COMPRESSION,
        raw_data.length].pack('SS x4 S2 L2 L x64 L'))

      output.write(raw_data)

      write_regions(output, meta[:regions], meta[:width], meta[:height])

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
