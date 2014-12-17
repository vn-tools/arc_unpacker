require 'stringio'
require_relative 'xp3_file_entry'

# XP3 file table
class Xp3FileTable
  attr_accessor :files

  def read!(file)
    raw = StringIO.new(read_raw_file_table!(file))
    @files = []
    @files.push(Xp3FileEntry.new.read!(raw)) until raw.eof?
    self
  end

  def read_raw_file_table!(file)
    use_zlib = file.read(1).unpack('B')[0]
    if use_zlib
      file_table_compressed_size = file.read(8).unpack('Q<')[0]
      file.seek(8, IO::SEEK_CUR) # 64 bit uncompressed file table size
      return Zlib.inflate(file.read(file_table_compressed_size))
    else
      file_table_size = file.read(8).unpack('Q')[0]
      return file.read(file_table_size)
    end
  end
end
