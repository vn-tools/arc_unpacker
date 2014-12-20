require_relative 'lzss_decoder'

# NSA file entry
class NsaFileEntry
  attr_reader :file_name
  attr_reader :compression_type
  attr_reader :data_origin
  attr_reader :data_size_compressed
  attr_reader :data_size_original

  def initialize(arc_data_origin)
    @data_offset = arc_data_origin
    @lzss_decoder = LzssDecoder.new
  end

  def read!(arc_file)
    @file_name = ''
    while (c = arc_file.read(1)) != "\0"
      @file_name += c
    end

    @compression_type,
    @data_origin,
    @data_size_compressed,
    @data_size_original = arc_file.read(13).unpack('CL>L>L>')
  end

  def read_data(arc_file)
    arc_file.seek(@data_offset + @data_origin, IO::SEEK_SET)
    data = unpack(arc_file.read(@data_size_compressed))
    fail 'Bad file size' unless data.length == @data_size_original
    data
  end

  def unpack(data)
    case @compression_type
    when 1
      fail \
        StandardError,
        'SPB compression not supported! Please send samples to rr- on github.'
    when 2
      return @lzss_decoder.decode(data)
    else
      return data
    end
  end
end
