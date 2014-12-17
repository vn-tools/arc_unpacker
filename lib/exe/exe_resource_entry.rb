# A single resource in file
class ExeResourceEntry
  attr_reader :file_name
  attr_reader :data_origin
  attr_reader :data_size

  def initialize(file_name, offset, size)
    @file_name = file_name
    @data_offset = offset
    @data_size = size
  end

  def read_data(input_file)
    input_file.seek(@data_offset, IO::SEEK_SET)
    input_file.read(@data_size)
  end
end
