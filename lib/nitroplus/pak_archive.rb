require 'stringio'
require 'zlib'
require_relative '../archive'
require_relative '../file_entry'

# PAK archive
class PakArchive < Archive
  MAGIC1 = "\x01\x00\x00\x00"
  MAGIC2 = "\x02\x00\x00\x00"
  MAGIC3 = "\x03\x00\x00\x00"
  MAGIC4 = "\x04\x00\x00\x00"

  def read_internal(arc_file)
    magic = arc_file.read(4)

    fail 'Not a PAK archive' \
      unless [MAGIC1, MAGIC2, MAGIC3, MAGIC4].include?(magic)

    if magic == MAGIC2
      read_pak2_file_table(arc_file)
    else
      fail 'Reading this PAK version is not yet supported.' \
        'Please send samples to rr- on github.'
    end
  end

  private

  def read_pak2_file_table(arc_file)
    file_count,
    table_size,
    compressed_table_size = arc_file.read(12).unpack('LLL')

    arc_file.seek(276, IO::SEEK_SET)
    raw = StringIO.new(Zlib.inflate(arc_file.read(compressed_table_size)))
    offset_to_files = arc_file.tell
    fail 'Bad file table size' unless raw.length == table_size

    @files = (1..file_count).map do
      read_file(raw, offset_to_files)
    end
  end

  def read_file(arc_file, offset_to_files)
    file_name = read_file_name(arc_file)

    data_origin,
    data_size_original,
    flags,
    data_size_compressed = arc_file.read(20).unpack('LLxxxxLL')

    data = lambda do
      arc_file.seek(data_origin + offset_to_files, IO::SEEK_SET)
      return Zlib.inflate(arc_file.read(data_size_compressed)) if flags > 0
      arc_file.read(data_size_original)
    end

    FileEntry.new(file_name, data)
  end

  def read_file_name(arc_file)
    file_name_length = arc_file.read(4).unpack('L')[0]
    file_name = arc_file.read(file_name_length)
    file_name.force_encoding('sjis').encode('utf-8')
  rescue
    file_name
  end
end
