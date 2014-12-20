require_relative '../../archive'
require_relative 'sar_file_table'

# SAR archive
class SarArchive < Archive
  def initialize
    @file_table = SarFileTable.new
  end

  def read(path)
    super
    open(path, 'rb') do |arc_file|
      @file_table.read!(arc_file)
    end
  end
end
