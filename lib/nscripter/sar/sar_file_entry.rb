# SAR file entry
class SarFileEntry
  attr_reader :file_name
  attr_reader :data

  def read!(arc_file, offset_to_files)
    @file_name = read_file_name(arc_file)

    data_origin,
    data_size = arc_file.read(8).unpack('L>L>')

    @data = lambda do |arc_file|
      arc_file.seek(offset_to_files + data_origin, IO::SEEK_SET)
      arc_file.read(data_size)
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
end
