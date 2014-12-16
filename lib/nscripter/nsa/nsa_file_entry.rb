require_relative 'lzss_decoder'

# NSA file entry
class NsaFileEntry
  attr_reader :file_name
  attr_reader :compression_type
  attr_reader :data_origin
  attr_reader :data_size_compressed
  attr_reader :data_size_original

  def initialize(file_data_origin)
    @data_offset = file_data_origin
    @lzss_decoder = LzssDecoder.new
  end

  def read!(file)
    @file_name = ''
    while (c = file.read(1)) != "\0"
      @file_name += c
    end

    @compression_type,
    @data_origin,
    @data_size_compressed,
    @data_size_original = file.read(13).unpack('CL>L>L>')
    self
  end

  def extract(input_file, target_path)
    input_file.seek(@data_offset + @data_origin, IO::SEEK_SET)
    data = unpack(input_file.read(@data_size_compressed))

    fail format('Expected %d bytes, got %d', data.length, @data_size_original) \
      if data.length != @data_size_original

    open(target_path, 'wb') { |output_file| output_file.write(data) }
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
