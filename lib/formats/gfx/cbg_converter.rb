require 'lib/image'
require_relative 'cbg_converter/cbg_pixel_decoder'

# Converts CBG to PNG.
# Engine: BGI
module CbgConverter
  module_function

  MAGIC = "CompressedBG___\x00"

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    Decoder.new.read(file.data).update_file(file)
  end

  def encode!(_file, _options)
    fail 'Not supported.'
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)

      magic = input.read(MAGIC.length)
      fail RecognitionError, 'Not a CBG picture' if magic != MAGIC

      width,
      height,
      bpp = input.read(8).unpack('SSL')
      input.skip(8)

      source_buffer = input.read
      target_buffer = decode_cbg_pixels(
        width,
        height,
        bpp,
        source_buffer)

      case bpp
      when 24
        format = 'BGR'
      when 8
        format = 'I'
      when 32
        format = 'BGRA'
      end

      Image.from_pixels(width, height, target_buffer, format)
    end
  end
end
