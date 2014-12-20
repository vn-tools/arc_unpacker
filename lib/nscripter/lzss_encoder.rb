require_relative '../bit_stream'

# Customizable LZSS compressor / decompressor.
class LzssEncoder
  def initialize(
      position_bits: 8,
      length_bits: 4,
      min_match_length: 2,
      initial_dictionary_pos: 0)
    @position_bits = position_bits
    @length_bits = length_bits
    @min_match_length = min_match_length
    @initial_dictionary_pos = initial_dictionary_pos
  end

  def encode(input)
    bs = BitStream.new

    input_pos = 0
    dictionary_size = 1 << @position_bits
    max_match_length = (1 << @length_bits) - 1
    dictionary_pos = @initial_dictionary_pos
    dictionary = "\x00" * dictionary_size

    while input_pos < input.length
      pos = nil
      best_match = nil
      (@min_match_length..max_match_length).reverse_each do |match_length|
        match_to_search = input[input_pos..input_pos + match_length - 1]
        pos = dictionary.index(match_to_search)
        next if match_to_search.length < @min_match_length
        next if pos.nil?
        best_match = match_to_search
        break
      end

      if best_match.nil?
        bs.write(1, 1)
        bs.write(input[input_pos].ord, 8)
        dictionary[dictionary_pos] = input[input_pos]
        dictionary_pos += 1
        dictionary_pos &= (dictionary_size - 1)
        input_pos += 1
      else
        bs.write(0, 1)
        bs.write(pos + @initial_dictionary_pos, @position_bits)
        bs.write(best_match.length - @min_match_length, @length_bits)
        (1..best_match.length).each do
          dictionary[dictionary_pos] = input[input_pos]
          dictionary_pos += 1
          dictionary_pos &= (dictionary_size - 1)
          input_pos += 1
        end
      end
    end
    bs.bytes
  end

  def decode(input)
    bs = BitStream.new(input)

    output = ''
    dictionary_size = 1 << @position_bits
    dictionary_pos = @initial_dictionary_pos
    dictionary = Array(1..dictionary_size).fill(0)

    until bs.eof?
      flag = bs.read(1)
      break if flag.nil?

      if flag & 1 == 1
        byte = bs.read(8)
        break if byte.nil?
        byte = byte.chr
        output << byte
        dictionary[dictionary_pos] = byte
        dictionary_pos += 1
        dictionary_pos &= (dictionary_size - 1)
      else
        pos = bs.read(@position_bits)
        break if pos.nil?
        length = bs.read(@length_bits)
        break if length.nil?
        length += @min_match_length
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
