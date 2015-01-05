require 'lib/formats/gfx/prs_converter'

# MBL archive
# Company: Ivory
# Engine: MarbleEngine
# Extension: .mbl
# Known games:
# - Wanko to Kurasou
module MblArchive
  def register_options(arg_parser, options)
    arg_parser.on(
      nil,
      '--version',
      'Which version to pack MBL archive with.',
      %(1 2)) do |version|
      options[:version] = version.to_i
    end
  end

  class Unpacker
    def unpack(arc_file, output_files, _options)
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
    rescue
      raise ArcError, 'Not a MBL archive'
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
          data = decode(data)
          [e[:name], data]
        end
      end
    end

    def decode(data)
      return PrsConverter.decode(data) if data.start_with?(PrsConverter::MAGIC)
      data
    end
  end

  class Packer
    def pack(arc_file, input_files, options)
      version = options[:version] || 2
      @arc_file = arc_file
      name_length = calc_name_length(input_files, version)
      write_dummy_table(input_files, version, name_length)
      table = write_contents(input_files)
      @arc_file.seek(0)
      write_table(table, version, name_length)
    end

    private

    def calc_name_length(input_files, version)
      return 16 if version == 1
      input_files.names.map { |n| n.encode('sjis').b.length }.max + 1
    end

    def write_dummy_table(input_files, version, name_length)
      table_size = 4
      table_size += 4 if version == 2
      table_size += (name_length + 8) * input_files.length
      @arc_file.write("\x00" * table_size)
    end

    def write_table(table, version, name_length)
      @arc_file.write([table.length].pack('L'))
      @arc_file.write([name_length].pack('L')) if version == 2
      table.each do |e|
        fail ArcError, 'Too long file name!' if e[:name].length > name_length
        @arc_file.write("\x00" * name_length)
        @arc_file.peek(@arc_file.tell - name_length) do
          @arc_file.write(e[:name].encode('sjis'))
        end
        @arc_file.write([e[:origin], e[:size]].pack('LL'))
      end
    end

    def write_contents(input_files)
      table = []
      input_files.each do |name, data|
        data = encode(data)
        table.push(name: name, origin: @arc_file.tell, size: data.length)
        @arc_file.write(data)
      end
      table
    end

    def encode(data)
      data = PrsConverter.encode(data) if data[1..3] == 'PNG'
      data
    end
  end
end
