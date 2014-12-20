require_relative 'sar_file_entry'

# SAR file table
class SarFileTable
  attr_reader :files
  attr_reader :file_data_origin

  def initialize
    @files = []
  end

  def read!(file)
    num_files,
    @file_data_origin = file.read(6).unpack('S>L>')

    @files = (1..num_files).map do
      entry = SarFileEntry.new(@file_data_origin)
      entry.read!(file)
      entry
    end
  end
end
