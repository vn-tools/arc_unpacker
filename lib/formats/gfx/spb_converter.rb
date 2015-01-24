require_relative 'spb_converter/spb_pixel_decoder'
require 'lib/binary_io'
require 'lib/image'

# Converts SPB to PNG.
# Seen in NSA archives.
module SpbConverter
  module_function

  MAGIC = "YB\x83\x03".b

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    image = Decoder.new.read(file.data)
    image.update_file(file)
  end

  def encode!(_file, _options)
    fail 'Not supported.'
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)
      width, height = input.read(4).unpack('S>S>')

      source_buffer = input.read
      target_buffer = spb_decode_pixels(
        source_buffer,
        width,
        height)

      Image.from_pixels(width, height, target_buffer, 'RGB')
    end
  end
end
