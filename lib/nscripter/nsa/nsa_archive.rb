require_relative '../../archive'
require_relative 'nsa_file_table'

# NSA archive
class NsaArchive < Archive
  def initialize
    @file_table = NsaFileTable.new
  end

  def read(path)
    super
    open(path, 'rb') do |arc_file|
      @file_table.read!(arc_file)
    end
  end
end
