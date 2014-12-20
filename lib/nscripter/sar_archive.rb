require_relative '../archive'
require_relative '../file_entry'

# SAR archive
class SarArchive < Archive
  def read_internal(arc_file)
    num_files,
    offset_to_files = arc_file.read(6).unpack('S>L>')

    @files = (1..num_files).map do
      file_name = read_file_name(arc_file)

      data_origin,
      data_size = arc_file.read(8).unpack('L>L>')

      data = lambda do
        arc_file.seek(offset_to_files + data_origin, IO::SEEK_SET)
        arc_file.read(data_size)
      end

      FileEntry.new(file_name, data)
    end
  end

  def write_internal(arc_file)
    table_size = @files.map { |f| f.file_name.length + 9 }.reduce(0, :+)
    offset_to_files = 6 + table_size
    arc_file.write([@files.length, offset_to_files].pack('S>L>'))
    arc_file.write("\x00" * table_size)

    cur_data_origin = 0
    table_entries = []
    @files.each do |file_entry|
      data = file_entry.data.call
      data_size = data.length
      arc_file.write(data)
      table_entries.push([file_entry.file_name, cur_data_origin, data_size])
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
