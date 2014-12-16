# Basic XP3 header (not fully reversed)
class Xp3Header
  MAGIC = "XP3\r\n"

  attr_accessor :magic
  attr_accessor :version
  attr_accessor :file_table_origin

  def read!(file)
    @magic = file.read(5)
    fail 'Not an XP3 archive' unless @magic == MAGIC

    @version = file.read(4).unpack('I')[0]
    file.seek(2, IO::SEEK_CUR)
    @file_table_origin = file.read(4).unpack('I')[0]
    self
  end
end
