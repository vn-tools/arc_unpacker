# YKC file entry
class YkcFileEntry
  YKS_MAGIC = 'YKS001'
  YKG_MAGIC = 'YKG000'

  attr_reader :file_name_origin
  attr_reader :file_name_size
  attr_reader :file_name
  attr_reader :data_origin
  attr_reader :data_size

  def read!(file)
    @file_name_origin,
    @file_name_size,
    @data_origin,
    @data_size = file.read(16).unpack('L4')
    file.seek(4, IO::SEEK_CUR)

    read_file_name(file)
    self
  end

  def extract(input_file, target_path)
    input_file.seek(@data_origin, IO::SEEK_SET)
    data = input_file.read(@data_size)
    if data[0..(YKS_MAGIC.length - 1)] == YKS_MAGIC
      offset_to_text = data[0x20..0x23].unpack('L')[0]
      bytes = data[offset_to_text..-1].unpack('C*')
      (0..(bytes.length - 1)).each { |i| bytes[i] ^= 0xaa }
      data = data[0..(offset_to_text - 1)] + bytes.pack('C*')
    elsif data[0..(YKG_MAGIC.length - 1)] == YKG_MAGIC
      data[0x41..0x43] = 'PNG'
      data = data[0x40..-1]
      target_path += '.png'
    end

    open(target_path, 'wb') do |output_file|
      output_file.write(data)
    end
  end

  private

  def read_file_name(file)
    old_pos = file.tell
    file.seek(@file_name_origin, IO::SEEK_SET)
    @file_name = file.read(@file_name_size - 1)
    @file_name = @file_name.gsub('\\', '/')
    file.seek(old_pos, IO::SEEK_SET)
  end
end
