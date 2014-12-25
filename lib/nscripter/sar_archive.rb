require_relative '../archive'

# SAR archive
class SarArchive < Archive
  def unpack_internal(arc_file, output_files)
    num_files,
    offset_to_files = arc_file.read(6).unpack('S>L>')

    num_files.times do
      file_name = read_file_name(arc_file)

      data_origin,
      data_size = arc_file.read(8).unpack('L>L>')

      old_pos = arc_file.tell
      arc_file.seek(offset_to_files + data_origin, IO::SEEK_SET)
      data = arc_file.read(data_size)
      arc_file.seek(old_pos, IO::SEEK_SET)

      output_files.write(file_name, data)
    end
  end

  def pack_internal(arc_file, input_files, _options)
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

    arc_file.seek(6, IO::SEEK_SET)
    table_entries.each do |file_name, data_origin, data_size|
      write_file_name(arc_file, file_name)
      arc_file.write([data_origin, data_size].pack('L>L>'))
    end
  end

  private

  def read_file_name(arc_file)
    file_name = ''
    while (c = arc_file.read(1)) != "\0"
      file_name += c
    end
    file_name
  end

  def write_file_name(arc_file, file_name)
    arc_file.write(file_name.gsub('/', '\\'))
    arc_file.write("\0")
  end
end
