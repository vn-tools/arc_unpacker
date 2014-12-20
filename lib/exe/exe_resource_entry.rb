# A single resource in file
class ExeResourceEntry
  attr_reader :file_name
  attr_reader :data

  def initialize(file_name, data_origin, data_size)
    @file_name = file_name
    @data = lambda do |input_file|
      input_file.seek(data_origin, IO::SEEK_SET)
      input_file.read(data_size)
    end
  end
end
