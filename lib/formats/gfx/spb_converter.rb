require_relative 'spb_converter/spb_pixel_decoder'
require 'lib/binary_io'
require 'lib/warning_silencer'
silence_warnings { require 'rmagick' }

# Converts SPB to PNG.
# Seen in NSA archives.
module SpbConverter
  module_function

  MAGIC = "YB\x83\x03".b

  def decode(input)
    input = BinaryIO.from_string(input)
    image_width, image_height = input.read(4).unpack('S>S>')

    source_size = input.size
    source_buffer = input.read
    target_buffer = decode_spb_pixels(
      source_buffer,
      source_size,
      image_width,
      image_height)

    image = Magick::Image.new(image_width, image_height)
    image.import_pixels(0, 0, image_width, image_height, 'RGB', target_buffer)
    image.to_blob { self.format = 'BMP' }
  end

  def encode(_input)
    fail 'Not supported.'
  end
end

