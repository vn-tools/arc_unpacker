require 'zlib'
require 'stringio'
require_relative 'pak2_file_entry'

# PAK2 file table
class Pak2FileTable
  attr_reader :files

  def initialize
    @files = []
  end

  def read!(arc_file)
    file_count,
    table_size,
    compressed_table_size = arc_file.read(12).unpack('LLL')

    arc_file.seek(276, IO::SEEK_SET)
    raw = StringIO.new(Zlib.inflate(arc_file.read(compressed_table_size)))
    data_offset = arc_file.tell
    fail 'Bad file table size' unless raw.length == table_size

    @files = (1..file_count).map do
      entry = Pak2FileEntry.new(data_offset)
      entry.read!(raw)
      entry
    end
  end
end
