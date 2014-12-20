require 'stringio'
require_relative 'xp3_file_entry'

# XP3 file table
class Xp3FileTable
  attr_accessor :files

  def initialize
    @files = []
  end

  def read!(arc_file)
    raw = StringIO.new(read_raw_file_table!(arc_file))
    @files = []
    until raw.eof?
      entry = Xp3FileEntry.new
      entry.read!(raw)
      @files.push(entry)
    end
    @files
  end

  def read_raw_file_table!(arc_file)
    use_zlib = arc_file.read(1).unpack('B')[0]

    if use_zlib
      file_table_size_compressed,
      file_table_size_original = arc_file.read(16).unpack('Q<Q<')
      raw = Zlib.inflate(arc_file.read(file_table_size_compressed))
      fail 'Bad file size' unless raw.length == file_table_size_original
      return raw
    end

    raw_size = arc_file.read(8).unpack('Q')[0]
    arc_file.read(raw_size)
  end
end
