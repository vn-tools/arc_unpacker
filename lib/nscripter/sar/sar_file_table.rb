require_relative 'sar_file_entry'

# SAR file table
class SarFileTable
  attr_reader :files

  def initialize
    @files = []
  end

  def read!(arc_file)
    num_files,
    offset_to_files = arc_file.read(6).unpack('S>L>')

    @files = (1..num_files).map do
      entry = SarFileEntry.new
      entry.read!(arc_file, offset_to_files)
      entry
    end
  end
end
