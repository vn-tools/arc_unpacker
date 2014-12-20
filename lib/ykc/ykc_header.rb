# Basic YKC header (not fully reversed)
class YkcHeader
  MAGIC = 'YKC001'

  attr_reader :magic
  attr_reader :version
  attr_reader :file_table_origin
  attr_reader :file_table_size

  def read!(arc_file)
    @magic = arc_file.read(6)
    fail 'Not a YKC archive' unless @magic == MAGIC

    arc_file.seek(2, IO::SEEK_CUR)
    @version = arc_file.read(4).unpack('L')[0]

    arc_file.seek(4, IO::SEEK_CUR)
    @file_table_origin,
    @file_table_size = arc_file.read(8).unpack('LL')
  end
end
