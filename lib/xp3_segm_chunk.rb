# Xp3 SEGM chunk
class Xp3SegmChunk
  attr_accessor :flags
  attr_accessor :offset
  attr_accessor :original_size
  attr_accessor :compressed_size

  def self.read_list!(file)
    magic = file.read(4)
    fail 'Expected segment chunk' unless magic == 'segm'
    segm_chunks_size = file.read(8).unpack('Q<')[0]
    segm_chunks = StringIO.new(file.read(segm_chunks_size))

    @chunks = []
    @chunks.push(Xp3SegmChunk.new.read!(segm_chunks)) until segm_chunks.eof?
    @chunks
  end

  def read_data!(file)
    file.seek(@offset, IO::SEEK_SET)
    use_zlib = @flags & 7 == 1
    if use_zlib
      return Zlib.inflate(file.read(@compressed_size))
    else
      return file.read(@original_size)
    end
  end

  def read!(file)
    @flags = file.read(4).unpack('L<')[0]
    @offset = file.read(8).unpack('Q<')[0]
    @original_size = file.read(8).unpack('Q<')[0]
    @compressed_size = file.read(8).unpack('Q<')[0]
    self
  end
end
