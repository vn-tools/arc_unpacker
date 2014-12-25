require_relative '../binary_io'

# SGD raw data decompressor
class SgdDecompressor
  def self.decompress(input)
    input = BinaryIO.from_string(input)
    output = BinaryIO.from_string('')

    decode_alpha(input, output)
    decode_bgr(input, output)

    output.rewind
    output.read
  end

  def self.decode_alpha(input, output)
    len = input.read(4).unpack('L')[0]
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
  end

  def self.decode_bgr(input, output)
    output.rewind
    len = input.read(4).unpack('L')[0]
    while len > 0
      flag = input.read(1).ord
      len -= 1

      case flag & 0xc0
      when 0x80
        pixels = flag & 0x3f
        output.seek(-4, IO::SEEK_CUR)
        b, g, r = output.read(4).unpack('C3x')

        pixels.times do
          delta = input.read(2).unpack('S')[0]
          len -= 2

          if delta & 0x8000 > 0
            b += delta & 0x1f
            g += (delta >> 5) & 0x1f
            r += (delta >> 10) & 0x1f

            output.write([r, g, b].pack('CCC'))
            output.seek(1, IO::SEEK_CUR)
            next
          end

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
  end
end
