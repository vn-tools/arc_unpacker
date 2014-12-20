# SAR file entry
class SarFileEntry
  attr_reader :file_name
  attr_reader :data_origin
  attr_reader :data_size

  def initialize(file_data_origin)
    @data_offset = file_data_origin
  end

  def read!(arc_file)
    @file_name = ''
    while (c = arc_file.read(1)) != "\0"
      @file_name += c
    end

    @data_origin,
    @data_size = arc_file.read(8).unpack('L>L>')
  end

  def read_data(arc_file)
    arc_file.seek(@data_offset + @data_origin, IO::SEEK_SET)
    arc_file.read(@data_size)
  end
end
