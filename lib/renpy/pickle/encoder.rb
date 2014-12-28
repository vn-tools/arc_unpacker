module Pickle
  # The error thrown by the encoder.
  class EncoderError < StandardError
  end

  # The pickle encoder.
  class Encoder
    def encode(input)
      @output = BinaryIO.from_string('')
      handle_object(input)
      @output.write(OPCODES[:STOP])
      @output.rewind
      @output.read
    end

    def handle_object(input)
      if input == true
        handle_true

      elsif input == false
        handle_false

      elsif input.nil?
        handle_nil

      elsif input.is_a?(String)
        handle_string(input)

      elsif input.is_a?(Float)
        handle_float(input)

      elsif input.is_a?(Integer)
        handle_integer(input)

      elsif input.is_a?(Array)
        handle_array(input)

      elsif input.is_a?(Hash)
        handle_hash(input)

      else
        fail EncoderError, 'Unsupported object.'
      end
    end

    def handle_true
      @output.write(OPCODES[:NEWTRUE])
    end

    def handle_false
      @output.write(OPCODES[:NEWFALSE])
    end

    def handle_nil
      @output.write(OPCODES[:NONE])
    end

    def handle_float(a)
      @output.write(OPCODES[:FLOAT] + a.to_s + "\n")
    end

    def handle_integer(a)
      if a > 0xffff_ffff
        @output.write(OPCODES[:LONG] + a.to_s + "\n")
      else
        @output.write(OPCODES[:INT] + a.to_s + "\n")
      end
    end

    def handle_array(a)
      @output.write(OPCODES[:MARK])
      a.each { |v| handle_object(v) }
      @output.write(OPCODES[:LIST])
    end

    def handle_hash(a)
      @output.write(OPCODES[:MARK])
      a.each do |k, v|
        handle_object(k)
        handle_object(v)
      end
      @output.write(OPCODES[:DICT])
    end

    def handle_string(a)
      @output.write(OPCODES[:BINUNICODE])
      @output.write([a.b.length].pack('L'))
      @output.write((a + '').b)
    end
  end
end
