require 'zlib'

# PAK2 file entry
class Pak2FileEntry
  attr_reader :file_name
  attr_reader :flags
  attr_reader :data_offset
  attr_reader :data_origin
  attr_reader :data_size_original
  attr_reader :data_size_compressed

  def initialize(file_data_origin)
    @data_offset = file_data_origin
  end

  def read!(file)
    @file_name = read_file_name(file)
    @data_origin,
    @data_size_original,
    @flags,
    @data_size_compressed = file.read(20).unpack('LLxxxxLL')
    self
  end

  def read_data(input_file)
    input_file.seek(@data_origin + @data_offset, IO::SEEK_SET)
    return Zlib.inflate(input_file.read(@data_size_compressed)) if @flags > 0
    input_file.read(@data_size_original)
  end

  def read_file_name(file)
    file_name_length = file.read(4).unpack('L')[0]
    file_name = file.read(file_name_length)
    begin
      return file_name.force_encoding('sjis').encode('utf-8')
    rescue
      return file_name
    end
  end
end
