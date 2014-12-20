require 'stringio'

# Xp3 SEGM chunk
class Xp3SegmChunk
  MAGIC = 'segm'

  attr_accessor :flags
  attr_accessor :offset
  attr_accessor :original_size
  attr_accessor :compressed_size

  def self.read_list!(arc_file)
    magic = arc_file.read(MAGIC.length)
    fail 'Expected segment chunk' unless magic == MAGIC

    raw_size = arc_file.read(8).unpack('Q<')[0]
    raw = StringIO.new(arc_file.read(raw_size))

    @chunks = []
    until raw.eof?
      chunk = Xp3SegmChunk.new
      chunk.read!(raw)
      @chunks.push(chunk)
    end
    @chunks
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

  def read!(arc_file)
    @flags,
    @offset,
    @original_size,
    @compressed_size = arc_file.read(28).unpack('L<Q<Q<Q<')
  end
end
