# Allows to read individual bits.
class BitReader
  def initialize(input)
    @mask_for_get_bit = 0
    @src_for_get_bit = 0
    @input_pos = 0
    @input = input

    @bits_read = 0
    @bit_length = input.length * 8
  end

  def get_bits(bits_to_read)
    result = 0

    (1..bits_to_read).each do
      return nil if @bits_read + 1 > @bit_length
      @bits_read += 1

      if @mask_for_get_bit == 0
        @src_for_get_bit = @input[@input_pos].ord
        @mask_for_get_bit = 0x80
        @input_pos += 1
      end

      result <<= 1
      result += 1 if @src_for_get_bit & @mask_for_get_bit > 0
      @mask_for_get_bit >>= 1
    end

    result
  end
end
