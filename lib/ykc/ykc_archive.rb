require_relative '../archive'
require_relative 'ykc_header'
require_relative 'ykc_file_table'

# YKC archive
class YkcArchive < Archive
  def initialize
    @header = YkcHeader.new
    @file_table = YkcFileTable.new
  end

  def read(path)
    super
    open(path, 'rb') do |arc_file|
      @header.read!(arc_file)
      @file_table.read!(arc_file, @header)
    end
  end
end
