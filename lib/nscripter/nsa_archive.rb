require_relative '../archive'
require_relative '../file_entry'
require_relative 'lzss_encoder'

# NSA archive
class NsaArchive < Archive
  NO_COMPRESSION = 0
  SPB_COMPRESSION = 1
  LZSS_COMPRESSION = 2

  def read_internal(arc_file)
    num_files,
    offset_to_files = arc_file.read(6).unpack('S>L>')

    @files = (1..num_files).map do
      file_name = read_file_name(arc_file)

      compression_type,
      data_origin,
      data_size_compressed,
      data_size_original = arc_file.read(13).unpack('CL>L>L>')

      data = lambda do
        arc_file.seek(offset_to_files + data_origin, IO::SEEK_SET)
        data = decompress(arc_file.read(data_size_compressed), compression_type)
        fail 'Bad file size' unless data.length == data_size_original
        data
      end

      FileEntry.new(file_name, data)
    end
  end

  def write_internal(arc_file, options)
    table_size = @files.map { |f| f.file_name.length + 14 }.reduce(0, :+)
    offset_to_files = 6 + table_size
    arc_file.write([@files.length, offset_to_files].pack('S>L>'))
    arc_file.write("\x00" * table_size)

    compression_type = options[:compression] || NO_COMPRESSION
    cur_data_origin = 0
    table_entries = []
    @files.each do |file_entry|
      data_original = file_entry.data.call
      data_compressed = compress(data_original, compression_type)
      data_size_original = data_original.length
      data_size_compressed = data_compressed.length

      arc_file.write(data_compressed)

      table_entries.push([
        file_entry.file_name,
        cur_data_origin,
        data_size_original,
        data_size_compressed])

      cur_data_origin += data_size_compressed
    end

    arc_file.seek(6, IO::SEEK_SET)
    table_entries.each do |file_name, data_origin, orig_size, compressed_size|
      write_file_name(arc_file, file_name)

      arc_file.write([
        compression_type,
        data_origin,
        compressed_size,
        orig_size].pack('CL>L>L>'))
    end
  end

  private

  def read_file_name(arc_file)
    file_name = ''
    while (c = arc_file.read(1)) != "\0"
      file_name += c
    end
    file_name
  end

  def write_file_name(arc_file, file_name)
    arc_file.write(file_name.gsub('/', '\\'))
    arc_file.write("\0")
  end

  def compress(data, compression_type)
    case compression_type
    when SPB_COMPRESSION
      fail \
        StandardError,
        'SPB compression not supported! Please send samples to rr- on github.'
    when LZSS_COMPRESSION
      return lzss_encoder.encode(data)
    else
      return data
    end
  end

  def decompress(data, compression_type)
    case compression_type
    when SPB_COMPRESSION
      fail \
        StandardError,
        'SPB compression not supported! Please send samples to rr- on github.'
    when LZSS_COMPRESSION
      return lzss_encoder.decode(data)
    else
      return data
    end
  end

  def lzss_encoder
    LzssEncoder.new(initial_dictionary_pos: 239, reuse_compressed: true)
  end
end
