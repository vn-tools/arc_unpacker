require 'lib/binary_io'
require 'lib/image'
require_relative 'tlg_converter/tlg5_pixel_decoder'

# Converts TLG to PNG.
# Seen in XP3 archives.
module TlgConverter
  module_function

  MAGIC_TLG0 = "TLG0.0\x00sds\x1a"
  MAGIC_TLG5 = "TLG5.0\x00raw\x1a"
  MAGIC_TLG6 = "TLG6.0\x00raw\x1a"

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
      if data.start_with?(MAGIC_TLG0)
        input.seek(MAGIC_TLG0.length)
        return read_tlg0(input)
      elsif data.start_with?(MAGIC_TLG5)
        input.seek(MAGIC_TLG5.length)
        return read_tlg5(input)
      elsif data.start_with?(MAGIC_TLG6)
        input.seek(MAGIC_TLG6.length)
        return read_tlg6(input)
      else
        fail 'Not a TLG image.'
      end
    end

    def read_tlg0(_input)
      fail 'Not supported (yet)'
    end

    def read_tlg5(input)
      header = {}
      header[:channel_count],
      header[:image_width],
      header[:image_height],
      header[:block_height] = input.read(13).unpack('CL4')
      pixels = input.read

      output = decode_tlg5_pixels(header, pixels)
      Image.from_pixels(
        header[:image_width],
        header[:image_height],
        output,
        'RGBA')
    end

    def read_tlg6(_input)
      fail 'Not supported (yet)'
    end
  end
end
