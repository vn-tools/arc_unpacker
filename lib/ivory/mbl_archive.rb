require_relative '../archive'

# MBL archive
class MblArchive < Archive
  def unpack_internal(arc_file, output_files, _options)
    MblUnpacker.new.unpack(arc_file, output_files)
  end

  def pack_internal(arc_file, input_files, _options)
    MblPacker.new.pack(arc_file, input_files)
  end

  # MBL archive unpacker
  class MblUnpacker
    def unpack(arc_file, output_files)
      @arc_file = arc_file
      table = read_table
      unpack_files(table, output_files)
    end

    private

    def read_table
      table = []
      file_count, name_length = @arc_file.read(8).unpack('LL')
      file_count.times do
        name = @arc_file.peek(@arc_file.tell) { @arc_file.read_until_zero }
        @arc_file.skip(name_length)
        origin, size = @arc_file.read(8).unpack('LL')
        table.push(name: name, origin: origin, size: size)
      end
      table
    end

    def unpack_files(table, output_files)
      table.each do |e|
        output_files.write do
          data = @arc_file.peek(e[:origin]) { @arc_file.read(e[:size]) }
          [e[:name], data]
        end
      end
    end
  end

  # MBL archive packer
  class MblPacker
    def pack(arc_file, input_files)
      @arc_file = arc_file
      write_dummy_table(input_files)
      table = write_contents(input_files)
      @arc_file.seek(0)
      write_table(table)
    end

    private

    def write_dummy_table(input_files)
      name_length = input_files.names.map(&:length).max + 1
      table_size = 8 + (name_length + 8) * input_files.length
      @arc_file.write("\x00" * table_size)
    end

    def write_contents(input_files)
      table = []
      input_files.each do |name, data|
        table.push(name: name, origin: @arc_file.tell, size: data.length)
        @arc_file.write(data)
      end
      table
    end

    def write_table(table)
      name_length = table.map { |e| e[:name].length }.max + 1
      @arc_file.write([table.length, name_length].pack('LL'))
      table.each do |e|
        @arc_file.write(e[:name])
        @arc_file.write("\x00" * (name_length - e[:name].length))
        @arc_file.write([e[:origin], e[:size]].pack('LL'))
      end
    end
  end
end
