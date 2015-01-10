require 'lib/binary_io'
require 'lib/image'
require 'lib/binary_io'

# Converts G00 to PNG and vice versa.
# Seen in Key games:
# - Clannad
# - Little Busters
module G00Converter
  module_function

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode(data, _options)
    Decoder.new.decode(data)
  end

  def encode(_data, _options)
    fail 'Not supported'
  end

  class Decoder
    def decode(data)
      input = BinaryIO.from_string(data)

      header = read_header(input)

      case header[:version]
      when 0
        decode_version_0(input, header)
      when 1
        decode_version_1(input, header)
      when 2
        decode_version_2(input, header)
      else
        fail RecognitionError, 'Not a G00 image'
      end
    end

    private

    def read_header(input)
      header = {}
      header[:version],
      header[:width],
      header[:height] = input.read(5).unpack('CS2')
      header
    end

    def decode_version_0(input, header)
      compressed_size,
      uncompressed_size = input.read(8).unpack('L2')

      if compressed_size != input.size - 5
        fail RecognitionError, 'Bad compressed size'
      end
      if uncompressed_size != header[:width] * header[:height] * 4
        fail RecognitionError, 'Bad uncompressed size'
      end

      output = ''
      bs = BinaryIO.from_string(input.read)
      flag = bs.read(1).ord
      bit = 1
      until bs.eof?
        if bit == 256
          flag = bs.read(1).ord
          bit = 1
        end
        if flag & bit > 0
          output << bs.read(3)
        else
          tmp = bs.read(2).unpack('S<')[0] || 0
          look_behind = (tmp >> 4) * 3
          length = ((tmp & 0x0f) + 1) * 3
          length.times do
            output << output[-look_behind] unless output[-look_behind].nil?
          end
        end
        bit <<= 1
      end
      Image.raw_to_boxed(header[:width], header[:height], output, 'BGR')
    end

    def decode_version_1(_input, _header)
      fail 'Reading version 1 is not supported.'
      palette = {}
      raw_palette = input.read(256 * 4)
      while raw_palette.size > 0
        rgba = raw_palette.slice!(0, 4)
        palette[palette.length] = rgba
      end
      raw_data = ''
      input.read(width * height).unpack('C*').each do |b|
        raw_data << palette[b]
      end
      Image.raw_to_boxed(width, height, raw_data, 'ABGR')
    end

    def decode_version_2(_input, _header)
      fail 'Reading version 2 is not supported.'
    end
  end
end
