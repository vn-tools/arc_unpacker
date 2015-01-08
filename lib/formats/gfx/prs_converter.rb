require 'lib/formats/gfx/prs_converter/prs_pixel_decoder'
require 'lib/binary_io'
require 'lib/image'

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

  def encode(_input)
    fail 'Not supported.'
  end
end
