require_relative '../../archive'
require_relative 'melty_blood_header'
require_relative 'melty_blood_file_table'

# Melty Blood archive
class MeltyBloodArchive < Archive
  def read(path)
    super
    open(path, 'rb') do |file|
      @header = MeltyBloodHeader.new.read!(file)
      @file_table = MeltyBloodFileTable.new(@header).read!(file)
    end
  end
end
