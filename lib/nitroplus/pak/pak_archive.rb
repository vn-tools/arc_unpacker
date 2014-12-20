require_relative '../../archive'
require_relative 'pak_header'
require_relative 'pak2_file_table'

# PAK archive
class PakArchive < Archive
  def initialize
    @header = PakHeader.new
    @file_table = Pak2FileTable.new
  end

  def read(path)
    super
    open(path, 'rb') do |arc_file|
      @header.read!(arc_file)

      if @header.magic == PakHeader::MAGIC2
        @file_table.read!(arc_file)
      else
        fail 'Reading this PAK version is not yet supported.' \
          'Please send samples to rr- on github.'
      end
    end
  end
end
