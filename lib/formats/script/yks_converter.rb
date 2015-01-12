require 'lib/binary_io'
require 'json'

# Converts YKS to (almost) human readable representation and vice versa.
# Seen in YKC archives.
module YksConverter
  module_function

  # Each script is divided into four setions, that must be there in order for
  # game not to crash:
  # - "Entries"
  # - "Instructions"
  # - Some magic number
  # - Text

  # Text contains function names and text/binary function argument values.
  # Strings are NULL-separated.

  # The "entries" store pointers to the elements within "instructions". It
  # isn't used for compression - trying to replace it with [1,2,3,...] and
  # expanding instructions resulted in crashes. I honestly have no idea how
  # it's used, the way the game's code jumps over it seems almost random to me.

  # Instructions have following structure:
  # [DWORD1, DWORD2, DWORD3, DWORD4]
  # DWORD1 determines instruction type. It can be function name, parameter, or
  # a hardcoded function. DWORD2..4 change their meaning depending on DWORD1
  # and might be simple integers or offsets within text. Instead of duplicating
  # everything in this rant, it's better to look at the code below.

  # Finally, I have no idea what the magic number is, but it needs to be there.
  # It's not a checksum. It might be a number of variables so that the engine
  # can allocate sufficient memory, or a pointer to first instruction (stupid).

  # Even though there are things that I haven't figured out, like:
  #
  # - What is the purpose of "entries" and why can't it be flattened?
  # - Are there unhandled functions/operators whose DWORD2..4 refer to offsets
  #   within text, so that changing strings length will cause game to crash?
  # - Are there functions I missed?
  #
  # ...it should be sufficient to make translation possible.

  MAGIC = 'YKS001'
  OPCODE_FUNC = 0
  OPCODE_OPERATOR = 1
  OPCODE_INT_VALUE = 4
  OPCODE_STR_VALUE = 5
  OPCODE_GLOBAL_FLAG = 8
  OPCODE_STRING = 9
  OPCODE_COUNTER = 10

  def add_cli_help(arg_parser)
    arg_parser.add_help(
      '--encrypt-yks',
      'Enables encryption of YKS scripts.')
  end

  def parse_cli_options(arg_parser, options)
    options[:encrypt_yks] = arg_parser.flag?(%w(--encrypt))
  end

  def decode(data, _options)
    input = BinaryIO.from_string(data)

    magic = input.read(MAGIC.length)
    fail RecognitionError, 'Not a YKS script' if magic != MAGIC

    is_encrypted,
    _header_size,
    entries_origin,
    entry_count,
    code_origin,
    code_size,
    text_origin,
    text_size,
    unknown = input.read(48 - MAGIC.length).unpack('SL x4 L6 Q')
    is_encrypted = is_encrypted != 0

    text = input.peek(text_origin) { input.read(text_size) }
    text = xor(text) if is_encrypted

    entries = input.peek(entries_origin) do
      input.read(4 * entry_count).unpack('L*')
    end

    code = input.peek(code_origin) do
      input.read(16 * code_size).unpack('L*').each_slice(4).to_a
    end

    text = BinaryIO.from_string(text)
    opcodes = []
    code.each do |d1, d2, d3, d4|
      case d1
      when OPCODE_FUNC
        fail 'Unexpected value: ' + d4.to_s if d4 != 0
        opcodes << [
          'func_name',
          text.peek(d2) { text.read_until_zero },
          d3]

      when OPCODE_OPERATOR
        fail 'Unexpected value: ' + d4.to_s if d4 != 0xff_ff_ff_ff
        opcodes << [
          'operator',
          text.peek(d2) { text.read_until_zero },
          text.peek(d3) { text.read(4).unpack('L')[0] }]

      when OPCODE_INT_VALUE
        fail 'Unexpected value: ' + d2.to_s if d2 != 0
        fail 'Unexpected value: ' + d4.to_s if d4 != 0
        opcodes << [
          'int_value',
          text.peek(d3) { text.read(4).unpack('L')[0] }]

      when OPCODE_STR_VALUE
        fail 'Unexpected value: ' + d2.to_s if d2 != 0
        fail 'Unexpected value: ' + d4.to_s if d4 != 0
        value = text.peek(d3) do
          text.read_until_zero.force_encoding('sjis').encode('utf-8')
        end
        opcodes << ['str_value', value]

      when OPCODE_GLOBAL_FLAG
        fail 'Unexpected value: ' + d3.to_s if d3 != 0
        opcodes << [
          'func_global_flag',
          text.peek(d2) { text.read_until_zero },
          d4 - d2]

      when OPCODE_STRING
        fail 'Unexpected value: ' + d3.to_s if d3 != 0
        opcodes << [
          'func_string',
          text.peek(d2) { text.read_until_zero },
          d4 - d2]

      when OPCODE_COUNTER
        fail 'Unexpected value: ' + d2.to_s if d2 != 0
        fail 'Unexpected value: ' + d4.to_s if d4 != 0
        opcodes << ['counter', d3]

      else
        fail 'Unknown instruction: ' + d1.to_s

      end
    end

    [
      unknown,
      JSON.dump(entries),
      opcodes.map { |o| JSON.dump(o) } * "\n"
    ] * "\n".encode('binary')
  end

  def encode(data, options)
    data = data.split("\n")
    unknown = data.shift.to_i
    entries = JSON.load(data.shift)
    opcodes = data.map { |o| JSON.load(o) }

    code = BinaryIO.from_string
    text = BinaryIO.from_string
    entries = entries.map { |m| [m].pack('L') } * ''

    opcodes.each do |o|
      name = o.shift

      case name
      when 'func_name'
        pos = text.tell
        text.write(o[0])
        text.write("\x00")
        code << [OPCODE_FUNC, pos, o[1], 0].pack('L4')

      when 'operator'
        pos1 = text.tell
        text.write(o[0])
        text.write("\x00")
        pos2 = text.tell
        text.write([o[1]].pack('L'))
        code << [OPCODE_OPERATOR, pos1, pos2, 0xff_ff_ff_ff].pack('L4')

      when 'int_value'
        pos = text.tell
        text.write([o[0]].pack('L'))
        code << [OPCODE_INT_VALUE, 0, pos, 0].pack('L4')

      when 'str_value'
        pos = text.tell
        text.write(o[0].force_encoding('utf-8').encode('sjis'))
        text.write("\x00")
        code << [OPCODE_STR_VALUE, 0, pos, 0].pack('L4')

      when 'func_global_flag'
        pos = text.tell
        text.write(o[0])
        text.write("\x00")
        code << [OPCODE_GLOBAL_FLAG, pos, 0, pos + o[1]].pack('L4')

      when 'func_string'
        pos = text.tell
        text.write(o[0])
        text.write("\x00")
        code << [OPCODE_STRING, pos, 0, pos + o[1]].pack('L4')

      when 'counter'
        code << [OPCODE_COUNTER, 0, o[0], 0].pack('L4')

      else
        fail 'Unknown opcode name: ' + name
      end
    end

    text.rewind
    text = text.read
    text = xor(text) if options[:encrypt_yks]

    code.rewind
    code = code.read

    header_size = 48
    entries_origin = header_size
    code_origin = entries_origin + entries.length
    text_origin = code_origin + code.length

    output = BinaryIO.from_string
    output.write(MAGIC)
    output.write([
      options[:encrypt_yks] ? 1 : 0,
      header_size,
      entries_origin,
      entries.length / 4,
      code_origin,
      code.length / 16,
      text_origin,
      text.length,
      unknown].pack('SL x4 L6 Q'))

    output.write(entries)
    output.write(code)
    output.write(text)

    output.rewind
    output.read
  end

  def xor(data)
    data.unpack('C*').map { |b| b ^ 0xaa }.pack('C*')
  end
end
