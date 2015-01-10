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
      if compressed_size != input.size - input.tell + 8
        fail RecognitionError, 'Bad compressed size'
      end
      if uncompressed_size != header[:width] * header[:height] * 4
        fail RecognitionError, 'Bad uncompressed size'
      end

      output = decompress_version_0(input.read)
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

    def decode_version_2(input, header)
      region_count = input.read(4).unpack('L')[0]
      regions = []
      region_count.times do
        region = {}
        region[:x1],
        region[:y1],
        region[:x2],
        region[:y2],
        region[:ox],
        region[:oy] = input.read(24).unpack('L6')
        regions << region
      end

      compressed_size,
      uncompressed_size = input.read(8).unpack('L2')
      if compressed_size != input.size - input.tell + 8
        fail RecognitionError, 'Bad compressed size'
      end

      pix_input = decompress_version_1(input.read(uncompressed_size))
      pix_input = BinaryIO.from_string(pix_input)
      pix_output = BinaryIO.from_string('')
      pix_output.write("\x00" * header[:width] * header[:height] * 4)
      pix_output.rewind
      #fail 'Malformed data' if uncompressed_size < pix_input.length

      if pix_input.read(4).unpack('L')[0] != region_count
        fail RecognitionError, 'Bad bitmap count'
      end

      index = (1..region_count).map do
        entry = {}
        entry[:origin],
        entry[:size] = pix_input.read(8).unpack('LL')
        entry
      end

      index.each_with_index do |entry, i|
        region = regions[i]

        block = pix_input.peek(entry[:origin]) do
          BinaryIO.from_string(pix_input.read(entry[:size]))
        end
        block_type,
        part_count = block.read(4).unpack('S2')
        fail 'Unknown block type' unless block_type == 1

        block.skip(0x70)
        parts = (1..part_count).map do
          part = {}
          part[:x],
          part[:y],
          part[:tr],
          part[:width],
          part[:height] = block.read(10).unpack('S5')
          block.skip(0x52)

          x = part[:x] + region[:x1]
          y1 = part[:y] + region[:y1]
          y2 = y1 + part[:height]
          (y1..y2 - 1).each do |y|
            data = block.read(part[:width] * 4)
            pix_output.seek((x + y * header[:width]) * 4)
            pix_output.write(data)
          end
          part
        end

        entry[:region] = region
        entry[:parts] = parts
      end

      pix_output.rewind
      Image.raw_to_boxed(
        header[:width],
        header[:height],
        pix_output.read,
        'BGRA',
        regions: regions, index: index)
    end

    def decompress_version_0(input)
      decompress(input, 3, 1)
    end

    def decompress_version_1(input)
      decompress(input, 1, 2)
    end

    def decompress(input, byte_count, length_delta)
      output = ''
      bs = BinaryIO.from_string(input)
      flag = bs.read(1).ord
      bit = 1
      until bs.eof?
        if bit == 256
          flag = bs.read(1).ord
          bit = 1
        end
        if flag & bit > 0
          output << bs.read(byte_count)
        else
          tmp = (bs.read(2) || "\0\0").unpack('S<')[0]
          look_behind = (tmp >> 4) * byte_count
          length = ((tmp & 0x0f) + length_delta) * byte_count
          length.times do
            output << output[-look_behind] unless output[-look_behind].nil?
          end
        end
        bit <<= 1
      end
      output
    end
  end
end
