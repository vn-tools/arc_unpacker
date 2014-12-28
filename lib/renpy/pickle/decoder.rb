module Pickle
  # The error thrown by the decoder.
  class DecoderError < StandardError
  end

  # The pickle decoder.
  class Decoder
    def decode(input)
      @input = BinaryIO.from_string(input)
      @marker = Object.new
      @stack = []
      @memo = {}

      # Automagically figure what to call.
      handlers =
        Decoder.instance_methods
        .select { |m| m =~ /\Ahandle_/ }
        .map { |m| [m[/\Ahandle_(.+)\Z/, 1].upcase.to_sym, m] }
      handlers = Hash[handlers]

      until @input.eof?
        opcode_value = @input.read(1).b

        opcode_name = OPCODES.key(opcode_value)
        myfail('Unrecognized opcode: %s', opcode_value) if opcode_name.nil?

        handler = handlers[opcode_name]
        myfail('Unimplemented opcode: %s', opcode_value) if handler.nil?

        ret = send(handler)
        return ret if opcode_name == :STOP
      end
    end

    def marker_pos
      @stack.rindex(@marker)
    end

    def handle_mark
      @stack.push(@marker)
    end

    def handle_stop
      @stack.pop # implicit return
    end

    def handle_pop
      @stack.pop
    end

    def handle_pop_mark
      mp = marker_pos
      @stack = @stack[0..mp - 1]
    end

    def handle_dup
      @stack.push(@stack[-1])
    end

    def handle_float
      @stack.push(@input.readline.to_f)
    end

    def handle_int
      @stack.push(@input.readline.to_i)
    end

    def handle_binint1
      @stack.push(@input.read(1).unpack('c')[0])
    end

    def handle_binint2
      @stack.push(@input.read(2).unpack('s')[0])
    end

    def handle_binint4
      @stack.push(@input.read(4).unpack('l')[0])
    end

    def handle_long
      @stack.push(@input.readline.to_i)
    end

    def handle_none
      @stack.push(nil)
    end

    def handle_persid
      myfail('Not implemented')
    end

    def handle_binpersid
      myfail('Not implemented')
    end

    def handle_reduce
      myfail('Not implemented')
    end

    def handle_string
      quote = @input.read(1)
      str = ''
      loop do
        chr = @input.read(1)
        break if chr == quote
        if chr == '\\'
          chr = @input.read(1)
          case chr
          when 'x'
            str += [@input.read(2).to_i(16)].pack('U')
          when '\\'
            str += '\\'
          when '\''
            str += '\''
          when '"'
            str += '"'
          when 'n'
            str += "\n"
          when 't'
            str += "\t"
          when 'r'
            str += "\r"
          else
            myfail('Badly escaped literal: %s', chr)
          end
          next
        end
        str += chr
      end

      myfail('Expected newline') unless @input.read(1) == "\n"
      @stack.push(str)
    end

    def handle_binstring
      len = @input.read(4).unpack('L')[0]
      @stack.push(@input.read(len))
    end

    def handle_short_binstring
      len = @input.read(1).ord
      @stack.push(@input.read(len))
    end

    def handle_unicode
      str = ''
      loop do
        chr = @input.read(1)
        if chr == '\\'
          case @input.read(1)
          when 'x'
            str += [@input.read(2).to_i(16)].pack('U')
          when 'u'
            str += [@input.read(4).to_i(16)].pack('U')
          when 'U'
            str += [@input.read(8).to_i(16)].pack('U')
          when '\\'
            str += '\\'
          else
            myfail('Badly escaped literal: %s', chr)
          end
          next
        end
        break if chr == "\n"
        str += chr
      end
      @stack.push(str)
    end

    def handle_binunicode
      len = @input.read(4).unpack('L')[0]
      @stack.push(@input.read(len).force_encoding('utf-8'))
    end

    def handle_append
      value = @stack.pop
      @stack[-1].push(value)
    end

    def handle_build
      myfail('Not implemented')
    end

    def handle_global
      myfail('Not implemented')
    end

    def handle_dict
      mp = marker_pos
      dict = {}
      ((@stack.length - mp) / 2).times do
        value, key = @stack.pop, @stack.pop
        dict[key] = value
      end
      @stack.pop
      @stack.push(dict)
    end

    def handle_empty_dict
      @stack.push({})
    end

    def handle_appends
      myfail('Not implemented')
    end

    def handle_get
      myfail('Not implemented')
    end

    def handle_binget
      myfail('Not implemented')
    end

    def handle_long_binget
      myfail('Not implemented')
    end

    def handle_inst
      myfail('Not implemented')
    end

    def handle_list
      mp = marker_pos
      @stack = @stack[0..mp - 1] + [@stack[mp + 1..-1]]
    end

    def handle_empty_list
      @stack.push([])
    end

    def handle_obj
      myfail('Not implemented')
    end

    def handle_put
      i = @input.readline[0..-2].to_i
      myfail('Negative PUT argument') if i < 0
      @memo[i.to_s] = @stack[-1]
    end

    def handle_binput
      i = @input.read(1).ord
      @memo[i.to_s] = @stack[-1]
    end

    def handle_long_binput
      i = @input.read(4).unpack('i')
      @memo[i.to_s] = @stack[-1]
    end

    def handle_setitem
      value, key = @stack.pop, @stack.pop
      @stack[-1][key] = value
    end

    def handle_setitems
      mp = marker_pos
      dict = @stack[mp - 1]
      ((@stack.length - mp) / 2).times do
        value, key = @stack.pop, @stack.pop
        dict[key] = value
      end
      @stack.pop
    end

    def handle_binfloat
      myfail('Not implemented')
    end

    def handle_proto
      proto = @input.read(1).ord
      return if [0, 1, 2].include?(proto)
      myfail('Unsupported pickle protocol: %d', proto)
    end

    def handle_newobj
      myfail('Not implemented')
    end

    def handle_ext1
      myfail('Not implemented')
    end

    def handle_ext2
      myfail('Not implemented')
    end

    def handle_ext4
      myfail('Not implemented')
    end

    def handle_tuple
      mp = marker_pos
      @stack = @stack[0..mp - 1] + [@stack[mp + 1..-1]]
      @stack[-1].freeze
    end

    def handle_empty_tuple
      @stack.push([])
      @stack[-1].freeze
    end

    def handle_tuple1
      @stack[-1] = [@stack[-1]]
      @stack[-1].freeze
    end

    def handle_tuple2
      x, y = @stack.pop, @stack.pop
      @stack.push([y, x])
      @stack[-1].freeze
    end

    def handle_tuple3
      x, y, z = @stack.pop, @stack.pop, @stack.pop
      @stack.push([z, y, x])
      @stack[-1].freeze
    end

    def handle_newtrue
      @stack.push(true)
    end

    def handle_newfalse
      @stack.push(false)
    end

    def handle_long1
      len = @input.read(1).ord
      @stack.push((@input.read(len) + "\x00" * 7).unpack('Q<')[0])
    end

    def handle_long4
      myfail('Not implemented')
    end

    def myfail(message, *args)
      fail DecoderError, format(message, *args)
    end
  end
end
