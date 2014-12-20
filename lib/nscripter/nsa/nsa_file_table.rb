require_relative 'nsa_file_entry'

# NSA file table
class NsaFileTable
  attr_reader :files
  attr_reader :file_data_origin

  def initialize
    @files = []
  end

  def read!(arc_file)
    num_files,
    @file_data_origin = arc_file.read(6).unpack('S>L>')

    @files = (1..num_files).map do
      entry = NsaFileEntry.new(@file_data_origin)
      entry.read!(arc_file)
      entry
    end
  end
end
