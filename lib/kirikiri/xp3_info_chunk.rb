require 'stringio'

# XP3 INFO chunk
class Xp3InfoChunk
  attr_accessor :protect
  attr_accessor :original_file_size
  attr_accessor :compressed_file_size
  attr_accessor :file_name

  def read!(file)
    magic = file.read(4)
    fail 'Expected info chunk' unless magic == 'info'
    info_chunk_size = file.read(8).unpack('Q<')[0]
    info_chunk = StringIO.new(file.read(info_chunk_size))

    @protect = info_chunk.read(4).unpack('I<')[0]
    @original_file_size = info_chunk.read(8).unpack('Q<')[0]
    @compressed_file_size = info_chunk.read(8).unpack('Q<')[0]

    name_length = info_chunk.read(2).unpack('S<')[0]
    @file_name =
      info_chunk
      .read(name_length * 2)
      .force_encoding('utf-16le')
      .encode('utf-8')
    self
  end
end
