require_relative '../../archive'
require_relative 'sar_file_table'

# SAR archive
class SarArchive < Archive
  def read(path)
    super
    open(path, 'rb') do |file|
      @file_table = SarFileTable.new.read!(file)
    end
  end
end
