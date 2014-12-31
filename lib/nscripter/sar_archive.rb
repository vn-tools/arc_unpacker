# SAR archive
module SarArchive
  class Unpacker
    def unpack(arc_file, output_files, _options)
      table = read_table(arc_file)
      read_contents(arc_file, table, output_files)
    end

    def read_table(arc_file)
      num_files,
      offset_to_files = arc_file.read(6).unpack('S>L>')
      fail 'Bad offset to files' if offset_to_files > arc_file.size

      table = []
      num_files.times do
        e = {}

        e[:name] = arc_file.read_until_zero
        e[:origin],
        e[:size] = arc_file.read(8).unpack('L>L>')

        e[:origin] += offset_to_files
        table.push(e)

        fail 'Bad offset to file' if e[:origin] + e[:size] > arc_file.size
      end
      table
    end

    def read_contents(arc_file, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) { arc_file.read(e[:size]) }
          [e[:name], data]
        end
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, _options)
      table_size = input_files.names.map { |n| n.length + 9 }.reduce(0, :+)
      offset_to_files = 6 + table_size
      arc_file.write([input_files.length, offset_to_files].pack('S>L>'))
      arc_file.write("\x00" * table_size)

      cur_data_origin = 0
      table_entries = []
      input_files.reverse_each do |file_name, data|
        data_size = data.length
        arc_file.write(data)
        table_entries.push([file_name, cur_data_origin, data_size])
        cur_data_origin += data_size
      end

      arc_file.seek(6)
      table_entries.each do |file_name, data_origin, data_size|
        arc_file.write(file_name.gsub('/', '\\'))
        arc_file.write("\0")
        arc_file.write([data_origin, data_size].pack('L>L>'))
      end
    end
  end
end
