require_relative '../../archive'
require_relative 'nsa_file_table'

# NSA archive
class NsaArchive < Archive
  def read(path)
    super
    open(path, 'rb') do |file|
      @file_table = NsaFileTable.new.read!(file)
    end
  end
end
