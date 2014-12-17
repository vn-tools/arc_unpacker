require_relative '../archive'
require_relative 'ykc_header'
require_relative 'ykc_file_table'

# YKC archive
class YkcArchive < Archive
  def read(path)
    super
    open(path, 'rb') do |file|
      @header = YkcHeader.new.read!(file)
      @file_table = YkcFileTable.new.read!(file, @header)
    end
  end
end
