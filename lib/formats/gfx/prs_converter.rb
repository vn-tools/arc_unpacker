require 'lib/formats/gfx/prs_converter/prs_pixel_decoder'
require 'lib/binary_io'
require 'lib/warning_silencer'
silence_warnings { require 'rmagick' }

# Converts PRS to PNG.
# Seen in MBL archives.
module PrsConverter
  module_function

  MAGIC = "YB\x83\x03".b

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

  def encode(_input)
    fail 'Not supported.'
  end
end
