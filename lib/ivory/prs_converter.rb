require_relative 'prs_pixel_decoder'
require_relative '../warning_silencer'
require_relative '../binary_io'
silence_warnings { require 'rmagick' }

class PrsConverter
  MAGIC = "YB\x83\x03"

  def decode(input)
    input = BinaryIO.from_string(input)
    magic = input.read(4)
    fail 'Not a PRS graphic file' if magic != MAGIC

    source_size,
    image_width,
    image_height = input.read(12).unpack('Lx4S2')
    source_buffer = input.read(source_size)
    target_buffer = prs_decode_pixels(
      source_buffer,
      source_size,
      image_width,
      image_height)

    image = Magick::Image.new(image_width, image_height)
    image.import_pixels(0, 0, image_width, image_height, 'BGR', target_buffer)
    image.to_blob { self.format = 'PNG' }
  end

  def encode(input)
    fail 'Not supported.'
  end
end
