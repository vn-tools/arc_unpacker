require 'rmagick'
require_relative '../binary_io'

# Converts MGD to PNG and vice versa.
class MgdConverter
  MAGIC = 'MGD '

  NO_COMPRESSION = 0
  SGD_COMPRESSION = 1
  PNG_COMPRESSION = 2

  def self.encode(data)
    image = Magick::Image.from_blob(data)[0]
    blob = image.to_blob { |attrs| attrs.format = 'PNG' }
    blob_size = blob.length

    meta_data_size = 92
    output = BinaryIO.new
    output.write(MAGIC)
    output.write([meta_data_size, 1, 0].pack('SSL'))
    output.write([image.columns, image.rows].pack('S2'))
    output.write([blob_size, blob_size].pack('LL'))
    output.write([PNG_COMPRESSION].pack('L'))
    output.write("\0" * 36)
    output.write([0x40000000, 0, 2, 0].pack('LQLQ'))
    output.write([blob_size, 0].pack('L2'))
    output.write(blob)
    output.rewind
    output.read
  end

  def self.decode(data)
    input = BinaryIO.new(data)

    magic = input.read(MAGIC.length)
    fail 'Not a MGD picture' if magic != MAGIC

    _meta_data_size,
    _image_width,
    _image_height,
    _size_original,
    _size_compressed,
    compression_type,
    _data_size,
    _alpha_size = input.read(92).unpack('Sx6 S2 L2 L x28 L2')

    if compression_type == PNG_COMPRESSION
      return input.read
    elsif compression_type == NO_COMPRESSION
      fail 'Uncompressed'
    end

    fail 'Not supported'
  end
end
