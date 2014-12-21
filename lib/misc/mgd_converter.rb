require 'rmagick'
require_relative '../binary_io'

# Converts MGD to PNG and vice versa.
class MgdConverter
  MAGIC = 'MGD '

  NO_COMPRESSION = 0
  SGD_COMPRESSION = 1
  PNG_COMPRESSION = 2

  def self.decode(data)
    input = BinaryIO.new(data)

    magic = input.read(MAGIC.length)
    fail 'Not a MGD picture' if magic != MAGIC

    _data_offset,
    _format,
    image_width,
    image_height,
    _size_original,
    _size_compressed,
    compression_type = input.read(88).unpack('SS x4 S2 L2 S x66')

    case compression_type
    when PNG_COMPRESSION
      input.seek(4, IO::SEEK_CUR)
      return input.read
    when NO_COMPRESSION
      return uncompressed_to_png(input.read, image_width, image_height)
    when SGD_COMPRESSION
      return sgd_to_png(input.read, image_width, image_height)
    end

    fail 'Not supported?'
  end

  def self.encode(data)
    image = Magick::Image.from_blob(data)[0]
    png_blob = image.to_blob { |attrs| attrs.format = 'PNG' }
    png_size = png_blob.length

    data_offset = 92
    output = BinaryIO.new
    output.write(MAGIC)
    output.write([
      data_offset,
      1,
      image.columns,
      image.rows,
      png_size,
      png_size,
      PNG_COMPRESSION,
      png_size].pack('SS x4 S2 L2 S x66 L'))
    output.write(png_blob)
    output.rewind
    output.read
  end

  private

  def self.uncompressed_to_png(input, image_width, image_height)
    Magick::Image.from_blob(input) do
      self.format = 'RGBA'
      self.depth = 8
      self.size = "#{image_width}x#{image_height}"
    end.first.to_blob do
      self.format = 'PNG'
    end
  end

  def self.sgd_to_png(input, image_width, image_height)
    input = BinaryIO.new(input)
    output = BinaryIO.new

    compressed_length = input.read(4).unpack('L')[0]
    uncompressed_length = image_width * image_height * 4

    len = input.read(4).unpack('L')[0]

    # alpha
    while len > 0
      flag = input.read(2).unpack('S')[0]
      len -= 2
      if flag & 0x8000 > 0
        pixels = (flag & 0x7fff) + 1
        alpha = input.read(1).ord ^ 0xff
        abgr = "\x00\x00\x00" + alpha.chr
        len -= 1
        output << abgr * pixels
      else
        pixels = flag
        pixels.times do
          alpha = input.read(1).ord ^ 0xff
          output << "\x00\x00\x00" + alpha.chr
        end
        len -= pixels
      end
    end

    # bgr
    output.rewind
    output.seek(0, IO::SEEK_CUR) #skip very first alpha byte

    len = input.read(4).unpack('L')[0]
    while len > 0
      flag = input.read(1).ord
      len -= 1

      case flag & 0xc0
      when 0x80
        pixels = flag & 0x3f
        output.seek(-4, IO::SEEK_CUR)
        b, g, r = output.read(4).unpack('C3x')

        (1..pixels).each do
          delta = input.read(2).unpack('S')[0]
          len -= 2

          if delta & 0x8000 > 0
            b += delta & 0x1f
            g += (delta >> 5) & 0x1f
            r += (delta >> 10) & 0x1f
          else
            if delta & 0x0010 == 0
              b += delta & 0xf
            else
              b -= delta & 0xf
            end

            if delta & 0x0200 == 0
              g += (delta >> 5) & 0xf
            else
              g -= (delta >> 5) & 0xf
            end

            if delta & 0x4000 == 0
              r += (delta >> 10) & 0xf
            else
              r -= (delta >> 10) & 0xf
            end
          end

          output.write([r, g, b].pack('CCC'))
          output.seek(1, IO::SEEK_CUR)
        end

      when 0x40
        pixels = (flag & 0x3f) + 1
        b, g, r = input.read(3).unpack('CCC')
        len -= 3

        pixels.times do
          output.write([r, g, b].pack('CCC'))
          output.seek(1, IO::SEEK_CUR)
        end

      when 0
        pixels = flag
        pixels.times do
          b, g, r = input.read(3).unpack('CCC')
          len -= 3

          output.write([r, g, b].pack('CCC'))
          output.seek(1, IO::SEEK_CUR)
        end

      else
        fail 'You have been compressed badly.'

      end
    end

    output.rewind
    uncompressed_to_png(output.read, image_width, image_height)
  end

end
