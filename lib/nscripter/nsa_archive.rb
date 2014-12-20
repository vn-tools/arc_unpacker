require_relative '../archive'
require_relative '../file_entry'
require_relative 'lzss_decoder'

# NSA archive
class NsaArchive < Archive
  def initialize
    super
    @lzss_decoder = LzssDecoder.new
  end

  def read_internal(arc_file)
    num_files,
    offset_to_files = arc_file.read(6).unpack('S>L>')

    @files = (1..num_files).map do
      read_file(arc_file, offset_to_files)
    end
  end

  private

  def read_file(arc_file, offset_to_files)
    file_name = read_file_name(arc_file)

    compression_type,
    data_origin,
    data_size_compressed,
    data_size_original = arc_file.read(13).unpack('CL>L>L>')

    data = lambda do |arc_file|
      arc_file.seek(offset_to_files + data_origin, IO::SEEK_SET)
      data = unpack(arc_file.read(data_size_compressed), compression_type)
      fail 'Bad file size' unless data.length == data_size_original
      data
    end

    FileEntry.new(file_name, data)
  end

  def read_file_name(arc_file)
    file_name = ''
    while (c = arc_file.read(1)) != "\0"
      file_name += c
    end
    file_name
  end

  def unpack(data, compression_type)
    case compression_type
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
