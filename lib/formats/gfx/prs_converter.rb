require 'lib/formats/gfx/prs_converter/prs_pixel_decoder'
require 'lib/binary_io'
require 'lib/image'

# Converts PRS to PNG.
# Seen in MBL archives.
module PrsConverter
  module_function

  MAGIC = "YB\x83\x03".b

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    file.data = Decoder.new.read(file.data)
  end

  def encode!(_file, _options)
    fail 'Not supported.'
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)
      magic = input.read(4)
      fail RecognitionError, 'Not a PRS graphic file' if magic != MAGIC

      source_size,
      width,
      height = input.read(12).unpack('Lx4S2')
      source_buffer = input.read(source_size)
      target_buffer = prs_decode_pixels(
        source_buffer,
        source_size,
        width,
        height)

      Image.raw_to_boxed(width, height, target_buffer, 'BGR')
    end
  end
end
