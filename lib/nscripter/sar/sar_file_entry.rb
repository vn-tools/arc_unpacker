# SAR file entry
class SarFileEntry
  attr_reader :file_name
  attr_reader :data_origin
  attr_reader :data_size

  def initialize(file_data_origin)
    @data_offset = file_data_origin
  end

  def read!(file)
    @file_name = ''
    while (c = file.read(1)) != "\0"
      @file_name += c
    end

    @data_origin,
    @data_size = file.read(8).unpack('L>L>')
    self
  end

  def read_data(input_file)
    input_file.seek(@data_offset + @data_origin, IO::SEEK_SET)
    input_file.read(@data_size)
  end
end
