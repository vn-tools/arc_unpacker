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

  def read!(arc_file)
    @file_name = read_file_name(arc_file)
    @data_origin,
    @data_size_original,
    @flags,
    @data_size_compressed = arc_file.read(20).unpack('LLxxxxLL')
  end

  def read_data(arc_file)
    arc_file.seek(@data_origin + @data_offset, IO::SEEK_SET)
    return Zlib.inflate(arc_file.read(@data_size_compressed)) if @flags > 0
    arc_file.read(@data_size_original)
  end

  def read_file_name(arc_file)
    file_name_length = arc_file.read(4).unpack('L')[0]
    file_name = arc_file.read(file_name_length)
    file_name.force_encoding('sjis').encode('utf-8')
  rescue
    file_name
  end
end
