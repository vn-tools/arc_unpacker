require_relative '../archive'

# YKC archive
class YkcArchive < Archive
  MAGIC = 'YKC001'
  YKS_MAGIC = 'YKS001'
  YKG_MAGIC = 'YKG000'

  def unpack_internal(arc_file, output_files)
    magic = arc_file.read(MAGIC.length)
    fail 'Not a YKC archive' unless magic == MAGIC

    _version,
    file_table_origin,
    file_table_size = arc_file.read(18).unpack('xxLxxxxLL')

    arc_file.seek(file_table_origin)
    num_files = file_table_size / 20

    num_files.times { read_file(arc_file, output_files) }
  end

  private

  def read_file(arc_file, output_files)
    file_name_origin,
    file_name_size,
    data_origin,
    data_size = arc_file.read(20).unpack('L4 x4')

    file_name = read_file_name(arc_file, file_name_origin, file_name_size)

    old_pos = arc_file.tell
    arc_file.seek(data_origin, IO::SEEK_SET)
    data = arc_file.read(data_size)
    data = decode(data)
    arc_file.seek(old_pos, IO::SEEK_SET)

    output_files.write(file_name, data)
  end

  def read_file_name(arc_file, file_name_origin, file_name_size)
    old_pos = arc_file.tell
    arc_file.seek(file_name_origin, IO::SEEK_SET)
    file_name = arc_file.read(file_name_size - 1)
    file_name = file_name.gsub('\\', '/')
    arc_file.seek(old_pos, IO::SEEK_SET)
    file_name
  end

  def decode(data)
    if data[0..(YKS_MAGIC.length - 1)] == YKS_MAGIC
      offset_to_text = data[0x20..0x23].unpack('L')[0]
      bytes = data[offset_to_text..-1].unpack('C*')
      (0..(bytes.length - 1)).each { |i| bytes[i] ^= 0xaa }
      data = data[0..(offset_to_text - 1)] + bytes.pack('C*')

    elsif data[0..(YKG_MAGIC.length - 1)] == YKG_MAGIC
      data[0x41..0x43] = 'PNG'
      data = data[0x40..-1]
    end
    data
  end
end
