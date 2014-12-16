require_relative 'nsa_file_entry'

# NSA file table
class NsaFileTable
  attr_reader :files
  attr_reader :file_data_origin

  def read!(file)
    num_files = file.read(2).unpack('S>')[0]
    @file_data_origin = file.read(4).unpack('L>')[0]

    @files = []
    (1..num_files).each do
      @files.push(NsaFileEntry.new(@file_data_origin).read!(file))
    end
    self
  end
end
