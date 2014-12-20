require 'stringio'

# XP3 INFO chunk
class Xp3InfoChunk
  MAGIC = 'info'

  attr_accessor :protect
  attr_accessor :original_file_size
  attr_accessor :compressed_file_size
  attr_accessor :file_name

  def read!(arc_file)
    magic = arc_file.read(MAGIC.length)
    fail 'Expected info chunk' unless magic == MAGIC

    raw_size = arc_file.read(8).unpack('Q<')[0]
    raw = StringIO.new(arc_file.read(raw_size))

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
