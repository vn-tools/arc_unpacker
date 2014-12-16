require_relative 'sar_file_entry'

# SAR file table
class SarFileTable
  attr_reader :files
  attr_reader :file_data_origin

  def read!(file)
    num_files = file.read(2).unpack('S>')[0]
    @file_data_origin = file.read(4).unpack('L>')[0]

    @files = []
    (1..num_files).each do
      @files.push(SarFileEntry.new(@file_data_origin).read!(file))
    end
    self
  end
end
