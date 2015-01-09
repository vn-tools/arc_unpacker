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

  def decode(input, _options)
    input = BinaryIO.from_string(input)
    width, height = input.read(4).unpack('S>S>')

    source_size = input.size
    source_buffer = input.read
    target_buffer = decode_spb_pixels(
      source_buffer,
      source_size,
      width,
      height)

    Image.raw_to_boxed(width, height, target_buffer, 'RGB')
  end

  def encode(_input, _options)
    fail 'Not supported.'
  end
end
