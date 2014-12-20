require_relative '../../archive'
require_relative 'melty_blood_header'
require_relative 'melty_blood_file_table'

# Melty Blood archive
class MeltyBloodArchive < Archive
  def initialize
    @header = MeltyBloodHeader.new
    @file_table = MeltyBloodFileTable.new(@header)
  end

  def read(path)
    super
    open(path, 'rb') do |arc_file|
      @header.read!(arc_file)
      @file_table.read!(arc_file)
    end
  end
end
