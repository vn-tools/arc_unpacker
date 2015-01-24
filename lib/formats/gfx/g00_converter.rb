require_relative 'g00_converter/g00_decompressor'
require 'lib/binary_io'
require 'lib/image'

# Converts G00 to PNG and vice versa.
# Seen in Key games:
# - Clannad
# - Little Busters
module G00Converter
  module_function

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    image = Decoder.new.read(file.data)
    image.update_file(file)
  end

  def encode!(_file, _options)
    fail 'Not supported'
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)
      header = read_header(input)

      case header[:version]
      when 0
        read_version_0(input, header)
      when 1
        read_version_1(input, header)
      when 2
        read_version_2(input, header)
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

    def read_version_0(input, header)
      compressed_size,
      uncompressed_size = input.read(8).unpack('L2')
      if compressed_size != input.size - input.tell + 8
        fail RecognitionError, 'Bad compressed size'
      end
      if uncompressed_size != header[:width] * header[:height] * 4
        fail RecognitionError, 'Bad uncompressed size'
      end

      output = decompress_version_0(
        input.read(compressed_size),
        uncompressed_size)
      output = output[0..header[:width] * header[:height] * 3 - 1]
      Image.from_pixels(header[:width], header[:height], output, 'BGR')
    end

    def read_version_1(input, header)
      compressed_size,
      uncompressed_size = input.read(8).unpack('L2')
      if compressed_size != input.size - input.tell + 8
        fail RecognitionError, 'Bad compressed size'
      end

      pix_input = BinaryIO.from_string(decompress_version_1(
        input.read(compressed_size),
        uncompressed_size))

      color_count = pix_input.read(2).unpack('S')[0]
      palette = {}
      color_count.times do
        palette[palette.length] = pix_input.read(4)
      end

      raw_data = ''
      pix_input.read(header[:width] * header[:height]).unpack('C*').each do |b|
        raw_data << palette[b]
      end
      Image.from_pixels(header[:width], header[:height], raw_data, 'BGRA')
    end

    def read_version_2(input, header)
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

      pix_input = input.read(compressed_size)
      pix_input = decompress_version_1(pix_input, uncompressed_size)
      pix_input = BinaryIO.from_string(pix_input)
      fail 'Malformed data' if uncompressed_size != pix_input.length

      pix_output = BinaryIO.from_string('')
      pix_output.write("\x00" * header[:width] * header[:height] * 4)
      pix_output.rewind

      if pix_input.read(4).unpack('L')[0] != region_count
        fail RecognitionError, 'Bad bitmap count'
      end

      index = []
      region_count.times do
        entry = {}
        entry[:origin],
        entry[:size] = pix_input.read(8).unpack('Ll')
        index << entry
      end

      index.each_with_index do |entry, i|
        region = regions[i]

        if entry[:size] > 0
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
        end

        entry[:region] = region
        entry[:parts] = parts
      end

      pix_output.rewind
      raw_data = pix_output.read
      meta = { regions: regions, index: index }
      Image.from_pixels(header[:width], header[:height], raw_data, 'BGRA', meta)
    end

    def decompress_version_0(input, output_size)
      decompress_g00(input, output_size, 3, 1)
    end

    def decompress_version_1(input, output_size)
      decompress_g00(input, output_size, 1, 2)
    end
  end
end
