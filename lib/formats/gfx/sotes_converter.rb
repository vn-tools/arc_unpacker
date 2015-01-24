require 'lib/binary_io'
require 'lib/image'

# Converts Fortune Summoners sprites to PNG.
# Engine: Unknown
# Known games:
# - Fortune Summoners: Secret of the Elemental Stone
module SotesConverter
  module_function

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    Decoder.new.read(file.data).update_file(file)
  end

  def encode!(_file, _options)
    fail 'Not supported'
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)
      fail RecognitionError, 'Not an image' if input.size < 1112
      weird_data1 = input.read(32).unpack('L*')
      palette = input.read(256 * 4).chunks(4)
      weird_data2 = input.read(56).unpack('L*')
      pixel_data_offset = weird_data2[12] - weird_data2[10]
      fail RecognitionError, 'Not an image' if pixel_data_offset >= input.size
      input.skip(pixel_data_offset)
      pixel_data = input.read

      width = guess_image_dimension(
        weird_data1[1..5],
        - weird_data1[6],
        [0, 1, 2, 3],
        pixel_data.length)

      height = guess_image_dimension(
        weird_data2[0..5],
        - weird_data2[10],
        [0],
        pixel_data.length)

      use_palette = width * height * 3 != pixel_data.length
      pixel_data = mirror(pixel_data, use_palette ? width : 3 * width)
      if use_palette
        pixel_data = pixel_data.unpack('C*').map { |i| palette[i] }.join
        return Image.from_pixels(width, height, pixel_data, 'BGRA')
      end

      Image.from_pixels(width, height, pixel_data, 'BGR')
    end

    def mirror(input, scanline_width)
      input = BinaryIO.from_string(input)
      output = BinaryIO.from_string
      (0..input.size / scanline_width - 1).reverse_each do |y|
        input.peek(y * scanline_width) do
          output.write(input.read(scanline_width))
        end
      end
      output.rewind
      output.read
    end

    def guess_image_dimension(candidates, main_delta, extra_deltas, pixels_size)
      candidates.each do |base|
        extra_deltas.each do |delta|
          possible_dimension = base + delta + main_delta
          next if possible_dimension <= 0
          return possible_dimension if pixels_size % possible_dimension == 0
        end
      end
      fail RecognitionError, 'Cannot figure out the image dimensions'
    end
  end
end
