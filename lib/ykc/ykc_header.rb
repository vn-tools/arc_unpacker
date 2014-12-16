# Basic YKC header (not fully reversed)
class YkcHeader
  MAGIC = 'YKC001'

  attr_reader :magic
  attr_reader :version
  attr_reader :file_table_origin
  attr_reader :file_table_size

  def read!(file)
    @magic = file.read(6)
    fail 'Not a YKC archive' unless @magic == MAGIC

    file.seek(2, IO::SEEK_CUR)
    @version = file.read(4).unpack('L')[0]

    file.seek(4, IO::SEEK_CUR)
    @file_table_origin,
    @file_table_size = file.read(8).unpack('LL')
    self
  end
end
