require 'zlib'
require_relative '../binary_io'
require_relative '../archive'
require_relative '../file_entry'

# XP3 archive
class Xp3Archive < Archive
  MAGIC = "XP3\r\n"
  FILE_MAGIC = 'File'
  ADLR_MAGIC = 'adlr'
  INFO_MAGIC = 'info'
  SEGM_MAGIC = 'segm'

  def initialize(decryptor)
    super
    @decryptor = decryptor
  end

  def read_internal(arc_file)
    magic = arc_file.read(5)
    fail 'Not an XP3 archive' unless magic == MAGIC

    _version,
    file_table_origin = arc_file.read(10).unpack('IxxI')

    arc_file.seek(file_table_origin, IO::SEEK_SET)

    raw = BinaryIO.new(read_raw_file_table!(arc_file))
    @files = []
    @files.push(read_file(raw)) until raw.eof?
  end

  private

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

  def read_file(arc_file)
    magic = arc_file.read(FILE_MAGIC.length)
    fail 'Expected file chunk' unless magic == FILE_MAGIC

    raw_size = arc_file.read(8).unpack('Q<')[0]
    raw = BinaryIO.new(arc_file.read(raw_size))

    info_chunk = Xp3InfoChunk.new
    info_chunk.read!(raw)
    segm_chunks = Xp3SegmChunk.read_list!(raw)
    adlr_chunk = Xp3AdlrChunk.new
    adlr_chunk.read!(raw)

    data = lambda do
      data = segm_chunks.map { |segm| segm.read_data!(arc_file) } * ''
      @decryptor.filter(data, adlr_chunk)
    end

    FileEntry.new(info_chunk.file_name, data)
  end

  # Xp3 SEGM chunk
  class Xp3SegmChunk
    def self.read_list!(arc_file)
      magic = arc_file.read(SEGM_MAGIC.length)
      fail 'Expected segment chunk' unless magic == SEGM_MAGIC

      raw_size = arc_file.read(8).unpack('Q<')[0]
      raw = BinaryIO.new(arc_file.read(raw_size))

      chunks = []
      until raw.eof?
        chunk = Xp3SegmChunk.new
        chunk.read!(raw)
        chunks.push(chunk)
      end
      chunks
    end

    def read!(arc_file)
      @flags,
      @offset,
      @original_size,
      @compressed_size = arc_file.read(28).unpack('L<Q<Q<Q<')
    end

    def read_data!(arc_file)
      arc_file.seek(@offset, IO::SEEK_SET)
      use_zlib = @flags & 7 == 1
      if use_zlib
        raw = Zlib.inflate(arc_file.read(@compressed_size))
        fail 'Bad SEGM size' unless raw.length == @original_size
        raw
      end

      arc_file.read(@original_size)
    end
  end

  # XP3 INFO chunk
  class Xp3InfoChunk
    attr_accessor :protect
    attr_accessor :original_file_size
    attr_accessor :compressed_file_size
    attr_accessor :file_name

    def read!(arc_file)
      magic = arc_file.read(INFO_MAGIC.length)
      fail 'Expected info chunk' unless magic == INFO_MAGIC

      raw_size = arc_file.read(8).unpack('Q<')[0]
      raw = BinaryIO.new(arc_file.read(raw_size))

      @protect,
      @original_file_size,
      @compressed_file_size,
      name_length = raw.read(22).unpack('I<Q<Q<S<')

      @file_name =
        raw
        .read(name_length * 2)
        .force_encoding('utf-16le')
        .encode('utf-8')
    end
  end

  # XP3 ADLR chunk
  class Xp3AdlrChunk
    attr_accessor :encryption_key

    def read!(arc_file)
      magic = arc_file.read(ADLR_MAGIC.length)
      fail 'Expected ADLR chunk' unless magic == ADLR_MAGIC

      raw_size = arc_file.read(8).unpack('Q<')[0]
      raw = BinaryIO.new(arc_file.read(raw_size))

      @encryption_key = raw.read(4).unpack('L<')
    end
  end
end
