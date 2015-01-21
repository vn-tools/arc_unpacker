require 'lib/binary_io'
require 'lib/image'
require 'zlib'

# Converts XYZ to PNG.
# Engine: RPGMaker
# Known games:
# - Yume Nikki
module XyzConverter
  module_function

  MAGIC = 'XYZ1'

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
      fail RecognitionError, 'Not a XYZ picture' if magic != MAGIC

      width, height = input.read(4).unpack('S2')

      input = BinaryIO.from_string(Zlib.inflate(input.read))
      palette = input.read(256 * 3).chunks(3)

      pixels = BinaryIO.from_string('')
      pixels.write(palette[input.read(1).ord]) until input.eof?
      pixels.rewind
      pixels = pixels.read

      Image.from_pixels(width, height, pixels, 'RGB')
    end
  end
end
