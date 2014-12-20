require 'zlib'
require_relative '../archive'
require_relative 'xp3_header'
require_relative 'xp3_file_table'

# XP3 archive
class Xp3Archive < Archive
  def initialize(decryptor)
    @decryptor = decryptor
    @header = Xp3Header.new
    @file_table = Xp3FileTable.new
  end

  def read(path)
    super
    open(path, 'rb') do |arc_file|
      @header.read!(arc_file)
      arc_file.seek(@header.file_table_origin, IO::SEEK_SET)
      @file_table.read!(arc_file)
    end
  end

  def read_data_from_file(file_entry, arc_file)
    file_entry.read_data(
      arc_file,
      ->(data) { @decryptor.filter(data, file_entry) })
  end
end
