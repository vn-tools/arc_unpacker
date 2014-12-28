require_relative '../archive'

# MBL archive
class MblArchive < Archive
  def unpack_internal(arc_file, output_files, _options)
    MblUnpacker.new.unpack(arc_file, output_files)
  end

  def pack_internal(arc_file, input_files, options)
    MblPacker.new.pack(arc_file, input_files, options[:version] || 2)
  end

  def register_options(arg_parser, options)
    arg_parser.on(
      nil,
      '--version',
      'Which version to pack MBL archive with.',
      %(1 2)) do |version|
      options[:version] = version.to_i
    end
  end

  # MBL archive unpacker
  class MblUnpacker
    def unpack(arc_file, output_files)
      @arc_file = arc_file
      version = detect_version
      table = read_table(version)
      unpack_files(table, output_files)
    end

    private

    def detect_version
      file_count = @arc_file.read(4).unpack('L')[0]
      @arc_file.skip(file_count * (16 + 8) - 8)
      last_file_origin, last_file_size = @arc_file.read(8).unpack('LL')
      return 1 if last_file_origin + last_file_size == @arc_file.size
      return 2
    ensure
      @arc_file.seek(0)
    end

    def read_table(version)
      table = []
      if version == 2
        file_count, name_length = @arc_file.read(8).unpack('LL')
      else
        file_count, name_length = @arc_file.read(4).unpack('L')[0], 16
      end
      file_count.times do
        name = @arc_file.peek(@arc_file.tell) { @arc_file.read_until_zero }
        name = name.force_encoding('sjis').encode('utf-8')
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
    def pack(arc_file, input_files, version)
      @arc_file = arc_file
      write_dummy_table(input_files, version)
      table = write_contents(input_files)
      @arc_file.seek(0)
      write_table(table, version)
    end

    private

    def write_dummy_table(input_files, version)
      table_size = 4
      if version == 2
        name_length = input_files.names.map(&:length).max + 1
        table_size += 4
      else
        name_length = 16
      end
      table_size += (name_length + 8) * input_files.length
      @arc_file.write("\x00" * table_size)
    end

    def write_table(table, version)
      @arc_file.write([table.length].pack('L'))
      if version == 2
        name_length = table.map { |e| e[:name].length }.max + 1
        @arc_file.write([name_length].pack('L'))
      else
        name_length = 16
      end
      table.each do |e|
        fail 'Too long file name!' if e[:name].length > name_length
        @arc_file.write(e[:name].encode('sjis'))
        @arc_file.write("\x00" * (name_length - e[:name].length))
        @arc_file.write([e[:origin], e[:size]].pack('LL'))
      end
    end

    def write_contents(input_files)
      table = []
      input_files.each do |name, data|
        table.push(name: name, origin: @arc_file.tell, size: data.length)
        @arc_file.write(data)
      end
      table
    end
  end
end
