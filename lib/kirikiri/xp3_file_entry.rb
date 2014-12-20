require 'stringio'
require_relative 'xp3_info_chunk'
require_relative 'xp3_segm_chunk'
require_relative 'xp3_adlr_chunk'

# XP3 file table entry
class Xp3FileEntry
  MAGIC = 'File'

  attr_reader :info_chunk
  attr_reader :segm_chunks
  attr_reader :adlr_chunk

  def initialize
    @info_chunk = Xp3InfoChunk.new
    @segm_chunks = []
    @adlr_chunk = Xp3AdlrChunk.new
  end

  def file_name
    @info_chunk.file_name
  end

  def read!(arc_file)
    magic = arc_file.read(MAGIC.length)
    fail 'Expected file chunk' unless magic == MAGIC

    raw_size = arc_file.read(8).unpack('Q<')[0]
    raw = StringIO.new(arc_file.read(raw_size))

    @info_chunk.read!(raw)
    @segm_chunks = Xp3SegmChunk.read_list!(raw)
    @adlr_chunk.read!(raw)
  end

  def read_data(handle, filter)
    data = ''
    @segm_chunks.each { |segm_chunk| data += segm_chunk.read_data!(handle) }
    data = filter.call(data)
    data
  end
end
