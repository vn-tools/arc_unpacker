require 'lib/binary_io'
require 'zlib'

# PAK2 archive
# Company: Nitroplus
# Extension: .pak
# Known games:
# - Saya no Uta
module Pak2NitroplusArchive
  MAGIC = "\x02\x00\x00\x00"
  # MAGIC1 = "\x01\x00\x00\x00"
  # MAGIC2 = "\x02\x00\x00\x00"
  # MAGIC3 = "\x03\x00\x00\x00"
  # MAGIC4 = "\x04\x00\x00\x00"

  class Unpacker
    def unpack(arc_file, output_files, _options)
      magic = arc_file.read(4)
      fail ArcError, 'Not a PAK archive' unless magic == MAGIC

      table = read_table(arc_file)
      read_contents(arc_file, table, output_files)
    end

    private

    def read_table(arc_file)
      file_count,
      table_size_original,
      table_size_compressed = arc_file.read(12).unpack('L3')

      arc_file.skip(0x104)
      raw_table = Zlib.inflate(arc_file.read(table_size_compressed))
      raw_table = BinaryIO.from_string(raw_table)
      unless raw_table.length == table_size_original
        fail ArcError, 'Bad file table size'
      end
      offset_to_files = arc_file.tell

      table = []
      file_count.times do
        e = { name: read_file_name(raw_table) }
        e[:origin],
        e[:size_original],
        e[:flags],
        e[:size_compressed] = raw_table.read(20).unpack('L2x4L2')
        e[:origin] += offset_to_files
        table << e
      end
      table
    end

    def read_contents(arc_file, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) do
            next arc_file.read(e[:size_original]) unless e[:flags] > 0
            next Zlib.inflate(arc_file.read(e[:size_compressed]))
          end
          fail ArcError, 'Bad file size' unless data.length == e[:size_original]

          [e[:name], data]
        end
      end
    end

    def read_file_name(arc_file)
      file_name_length = arc_file.read(4).unpack('L')[0]
      file_name = arc_file.read(file_name_length)
      file_name.force_encoding('sjis').encode('utf-8')
    rescue
      file_name
    end
  end

  class Packer
    def pack(arc_file, input_files, options)
      arc_file.write(MAGIC)

      table = prepare_table(input_files, options)
      header_pos = arc_file.tell
      write_dummy_header(arc_file)

      table_sizes = write_table(arc_file, table)
      arc_file.peek(header_pos) { write_header(arc_file, table, table_sizes) }

      write_contents(arc_file, table, input_files)
    end

    private

    def write_dummy_header(arc_file)
      arc_file.write("\x00" * 12)
      arc_file.write("\x00" * 4)
      arc_file.write("\x00" * 0x100)
    end

    def write_header(arc_file, table, table_sizes)
      arc_file.write([
        table.length,
        *table_sizes,
        100].pack('L4'))
      arc_file.write("\x00" * 0x100)
    end

    def prepare_table(input_files, options)
      origin = 0
      table = {}
      input_files.each do |name, data|
        e = { name: name, origin: origin, size_original: data.length }
        if options[:compressed]
          e[:flags] = 1
          e[:size_compressed] = Zlib.deflate(data).length
        else
          e[:flags] = 0
          e[:size_compressed] = data.length
        end
        table[name] = e
        origin += e[:size_compressed]
      end
      table
    end

    def write_table(arc_file, table)
      raw_table = BinaryIO.from_string('')
      table.each do |name, e|
        name = name.gsub(/\//, '\\').encode('sjis').b
        raw_table.write([name.length].pack('L'))
        raw_table.write(name)
        raw_table.write([
          e[:origin],
          e[:size_original],
          e[:flags],
          e[:size_compressed]].pack('L2x4L2'))
      end
      raw_table.rewind
      raw_table = raw_table.read
      table_size_original = raw_table.length
      raw_table = Zlib.deflate(raw_table)
      table_size_compressed = raw_table.length
      arc_file.write(raw_table)
      [table_size_original, table_size_compressed]
    end

    def write_contents(arc_file, table, input_files)
      input_files.each do |name, data|
        data = Zlib.deflate(data) if table[name][:flags] > 0
        arc_file.write(data)
      end
    end
  end
end
