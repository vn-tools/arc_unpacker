# XP3 ADLR chunk
class Xp3AdlrChunk
  attr_accessor :encryption_key

  def read!(file)
    magic = file.read(4)
    fail 'Expected ADLR chunk' unless magic == 'adlr'
    adlr_chunk_size = file.read(8).unpack('Q<')[0]
    adlr_chunk = StringIO.new(file.read(adlr_chunk_size))

    @encryption_key = adlr_chunk.read(4).unpack('L<')
    self
  end
end
