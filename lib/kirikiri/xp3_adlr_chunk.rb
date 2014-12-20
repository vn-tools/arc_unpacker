require 'stringio'

# XP3 ADLR chunk
class Xp3AdlrChunk
  MAGIC = 'adlr'

  attr_accessor :encryption_key

  def read!(arc_file)
    magic = arc_file.read(MAGIC.length)
    fail 'Expected ADLR chunk' unless magic == MAGIC

    raw_size = arc_file.read(8).unpack('Q<')[0]
    raw = StringIO.new(arc_file.read(raw_size))

    @encryption_key = raw.read(4).unpack('L<')
  end
end
