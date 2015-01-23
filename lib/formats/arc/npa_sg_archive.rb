require 'lib/binary_io'
require 'lib/common'
require 'lib/virtual_file'

# NPA archive
# Engine: Unknown
# Extension: .npa
# Known games:
# - Steins;Gate
module NpaSgArchive
  module_function

  KEY = hex_s_to_a('BD AA BC B4 AB B6 BC B4')

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  class Unpacker
    def unpack(arc_file, output_files, _options)
      table = read_table(arc_file)
      read_contents(arc_file, table, output_files)
    end

    def read_table(arc_file)
      header_size = arc_file.read(4).unpack('L')[0]
      fail RecognitionError, 'Bad header size' if header_size > arc_file.size
      raw_table = arc_file.read(header_size)
      raw_table = NpaSgArchive.crypt(raw_table)
      raw_table = BinaryIO.from_string(raw_table)
      num_files = raw_table.read(4).unpack('L')[0]
      fail RecognitionError, 'Too many files' if num_files > header_size
      table = []
      num_files.times do
        e = {}

        file_name_length = raw_table.read(4).unpack('L')[0]
        e[:name] = raw_table.read(file_name_length)
          .force_encoding('utf-16le')
          .encode('utf-8')

        e[:size],
        e[:origin] = raw_table.read(8).unpack('L2')
        fail 'Bad file origin' if e[:origin] > arc_file.size
        raw_table.skip(4)
        table << e
      end

      table
    end

    def read_contents(arc_file, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) { arc_file.read(e[:size]) }
          data = NpaSgArchive.crypt(data)
          VirtualFile.new(e[:name], data)
        end
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, _options)
      table_size = 4
      table = {}
      input_files.names.each do |name|
        e = { name: name.encode('utf-16le').b }
        table[name] = e
        table_size += e[:name].length + 16
      end

      arc_file.write([table_size].pack('L'))
      arc_file.write("\x00" * table_size)

      input_files.each do |file|
        table[file.name][:origin] = arc_file.tell
        table[file.name][:size] = file.data.length
        arc_file.write(NpaSgArchive.crypt(file.data))
      end

      raw_table = BinaryIO.from_string
      raw_table.write([input_files.length].pack('L'))
      table.values.each do |e|
        raw_table.write([e[:name].length].pack('L'))
        raw_table.write(e[:name])
        raw_table.write([e[:size], e[:origin], 0].pack('L3'))
      end
      raw_table.rewind
      raw_table = NpaSgArchive.crypt(raw_table.read)

      arc_file.seek(4)
      arc_file.write(raw_table)
    end
  end

  def crypt(input)
    @key ||= ''
    if @key.length < input.length
      @key = KEY * ((input.length + KEY.length) / KEY.length)
    end
    input = input.unpack('C*')
    input.length.times { |i| input[i] ^= @key[i] }
    input = input.pack('C*')
    input
  end
end
