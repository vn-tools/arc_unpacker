require_relative '../bit_reader'

# Customized LZSS decompressor.
class LzssDecoder
  def decode(input)
    @bit_reader = BitReader.new(input)

    output = ''
    dictionary_size = 256
    dictionary_pos = 239
    dictionary = Array(1..dictionary_size).fill(0)

    loop do
      flag = @bit_reader.get_bits(1)
      break if flag.nil?

      if flag & 1 == 1
        byte = @bit_reader.get_bits(8).chr
        output << byte
        dictionary[dictionary_pos] = byte
        dictionary_pos += 1
        dictionary_pos &= (dictionary_size - 1)
      else
        pos = @bit_reader.get_bits(8)
        break if pos.nil?
        length = @bit_reader.get_bits(4) + 2
        (1..length).each do
          byte = dictionary[pos]
          pos += 1
          pos &= (dictionary_size - 1)
          dictionary[dictionary_pos] = byte
          dictionary_pos += 1
          dictionary_pos &= (dictionary_size - 1)
          output << byte
        end
      end
    end

    output
  end
end
