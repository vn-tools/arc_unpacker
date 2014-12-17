require 'zlib'
require 'stringio'
require_relative 'pak2_file_entry'

# PAK2 file table
class Pak2FileTable
  attr_reader :files

  def read!(file)
    file_count,
    table_size,
    compressed_table_size = file.read(16).unpack('LLL')

    file.seek(276, IO::SEEK_SET)
    raw = Zlib.inflate(file.read(compressed_table_size))
    data_offset = file.tell
    fail 'Bad file table size' unless raw.length == table_size

    table = StringIO.new(raw)

    @files = []
    (1..file_count).each do
      @files.push(Pak2FileEntry.new(data_offset).read!(table))
    end

    self
  end
end
