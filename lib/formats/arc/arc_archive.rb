require 'lib/virtual_file'

# ARC archive
# Engine: BGI/Ethornell
# Extension: .arc
# Known games:
# - Higurashi No Naku Koro Ni
module ArcArchive
  module_function

  MAGIC = 'PackFile    '

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  class Unpacker
    def unpack(arc_file, output_files, _options)
      magic = arc_file.read(MAGIC.length)
      fail RecognitionError, 'Not an ARC archive' unless magic == MAGIC

      table = read_table(arc_file)
      read_contents(arc_file, table, output_files)
    end

    def read_table(arc_file)
      num_files = arc_file.read(4).unpack('L')[0]
      fail RecognitionError, 'Bad file count' if num_files * 32 > arc_file.size

      table = []
      num_files.times do
        e = {}

        e[:name] = arc_file.peek(arc_file.tell) { arc_file.read_until_zero }
        arc_file.skip(16)

        e[:origin],
        e[:size] = arc_file.read(8).unpack('LL')
        e[:origin] += + MAGIC.length + 4 + num_files * 32
        arc_file.skip(8)

        if e[:origin] + e[:size] > arc_file.size
          fail RecognitionError, 'Bad offset to file'
        end

        table.push(e)
      end
      table
    end

    def read_contents(arc_file, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) { arc_file.read(e[:size]) }
          VirtualFile.new(e[:name], data)
        end
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, _options)
      input_files.names.each do |name|
        fail 'File names can contain 16 characters max.' if name.length > 16
      end

      arc_file.write(MAGIC)
      table_pos = arc_file.tell
      write_dummy_table(arc_file, input_files.length)

      table = write_contents(arc_file, input_files)

      arc_file.seek(table_pos)
      write_table(arc_file, table)
    end

    def write_dummy_table(arc_file, file_count)
      arc_file.write("\x00" * 4)
      arc_file.write("\x00" * (file_count * 32))
    end

    def write_table(arc_file, table)
      arc_file.write([table.length].pack('L'))
      table.each do |e|
        arc_file.peek(arc_file.tell) { arc_file.write("\x00" * 16) }
        arc_file.peek(arc_file.tell) { arc_file.write(e[:name]) }
        arc_file.skip(16)
        arc_file.write([e[:origin], e[:size], 0, 0].pack('L4'))
      end
    end

    def write_contents(arc_file, input_files)
      table = []

      current_origin = 0
      input_files.each do |file|
        e = {}
        e[:origin] = current_origin
        e[:size] = file.data.length
        e[:name] = file.name
        arc_file.write(file.data)
        current_origin += e[:size]
        table << e
      end

      table
    end
  end
end
