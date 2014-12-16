require_relative '../../archive'
require_relative 'pak_header'
require_relative 'pak2_file_table'

# PAK archive
class PakArchive < Archive
  def read(path)
    super
    open(path, 'rb') do |file|
      @header = PakHeader.new.read!(file)

      if @header.magic == PakHeader::MAGIC2
        @file_table = Pak2FileTable.new.read!(file)
      else
        fail 'Reading this PAK version is not yet supported.' \
          'Please send samples to rr- on github.'
      end
    end
  end
end
