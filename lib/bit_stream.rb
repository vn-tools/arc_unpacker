require_relative 'binary_io'

# Allows to read and write individual bits.
class BitStream
  SEEK_SET = :set
  SEEK_CUR = :cur

  def initialize(input = '')
    @mask_for_get_bit = 0
    @src_for_get_bit = 0
    @buffer = BinaryIO.new(input)

    @bit_pos = 0
  end

  def bytes
    pos = @buffer.tell
    @buffer.seek(0)
    bytes = @buffer.read
    @buffer.seek(pos)
    bytes.force_encoding('binary')
  end

  def bit_length
    @buffer.length * 8
  end

  def eof?
    @bit_pos >= bit_length
  end

  def tell # rubocop:disable Style/TrivialAccessors
    @bit_pos
  end

  def seek(offset, type = SEEK_CUR)
    if type == SEEK_CUR
      @bit_pos += offset
    elsif type == SEEK_SET
      @bit_pos = offset
    else
      fail 'Wrong direction'
    end

    @buffer.seek(@bit_pos / 8)
    @mask_for_get_bit = 0x100 >> ((@bit_pos & 7) + 1)
    @src_for_get_bit = (@buffer.read(1) || "\x00").ord
  end

  def write(number, bits_to_write)
    return if bits_to_write < 1

    length = ((@bit_pos & 7) + bits_to_write + 7) / 8

    @buffer.seek(@bit_pos / 8)
    first_byte = (@buffer.read(1) || "\x00").ord
    @buffer.seek(length - 1)
    last_byte = (@buffer.read(1) || "\x00").ord
    @buffer.seek(@bit_pos / 8)

    first_byte_mask = ((1 << (8 - (@bit_pos & 7))) - 1) ^ 0xff
    last_byte_mask = (1 << (8 - (((@bit_pos + bits_to_write - 1) & 7) + 1))) - 1
    number <<= (8 - (((@bit_pos + bits_to_write - 1) & 7) + 1))

    bytes = []
    (0..length - 1).reverse_each do |i|
      bytes[i] = number & 0xff
      number >>= 8
    end

    bytes[0] &= 0xff ^ first_byte_mask
    bytes[0] |= first_byte & first_byte_mask
    bytes[-1] &= 0xff ^ last_byte_mask
    bytes[-1] |= last_byte & last_byte_mask

    @buffer.write(bytes.map(&:chr) * '')
    seek(bits_to_write)
  end

  def read(bits_to_read)
    result = 0

    (1..bits_to_read).each do
      return nil if @bit_pos + 1 > bit_length

      @bit_pos += 1

      if @mask_for_get_bit == 0
        @src_for_get_bit = @buffer.read(1).ord
        @mask_for_get_bit = 0x80
      end

      result <<= 1
      result += 1 if @src_for_get_bit & @mask_for_get_bit > 0

      @mask_for_get_bit >>= 1
    end

    result
  end
end
