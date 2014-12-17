require 'stringio'
require_relative 'xp3_info_chunk'
require_relative 'xp3_segm_chunk'
require_relative 'xp3_adlr_chunk'

# XP3 file table entry
class Xp3FileEntry
  attr_reader :info_chunk
  attr_reader :segm_chunks
  attr_reader :adlr_chunk

  def file_name
    @info_chunk.file_name
  end

  def read!(file)
    magic = file.read(4)
    fail 'Expected file chunk' unless magic == 'File'
    file_chunk_size = file.read(8).unpack('Q<')[0]
    file_chunk = StringIO.new(file.read(file_chunk_size))

    @info_chunk = Xp3InfoChunk.new.read!(file_chunk)
    @segm_chunks = Xp3SegmChunk.read_list!(file_chunk)
    @adlr_chunk = Xp3AdlrChunk.new.read!(file_chunk)
    self
  end

  def read_data(handle, filter)
    data = ''
    @segm_chunks.each { |segm_chunk| data += segm_chunk.read_data!(handle) }
    data = filter.call(data)
    data
  end
end
