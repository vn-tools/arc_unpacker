# YKC file entry
class YkcFileEntry
  YKS_MAGIC = 'YKS001'
  YKG_MAGIC = 'YKG000'

  attr_reader :file_name
  attr_reader :data

  def read!(arc_file)
    file_name_origin,
    file_name_size,
    data_origin,
    data_size = arc_file.read(16).unpack('L4')

    arc_file.seek(4, IO::SEEK_CUR)
    @file_name = read_file_name(arc_file, file_name_origin, file_name_size)

    @data = lambda do |arc_file|
      arc_file.seek(data_origin, IO::SEEK_SET)
      data = arc_file.read(data_size)

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

  private

  def read_file_name(arc_file, file_name_origin, file_name_size)
    old_pos = arc_file.tell
    arc_file.seek(file_name_origin, IO::SEEK_SET)
    file_name = arc_file.read(file_name_size - 1)
    file_name = file_name.gsub('\\', '/')
    arc_file.seek(old_pos, IO::SEEK_SET)
    file_name
  end
end
