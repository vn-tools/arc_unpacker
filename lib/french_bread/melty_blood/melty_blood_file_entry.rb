# Melty Blood file entry
class MeltyBloodFileEntry
  ENCRYPTION_KEY = 0xE3DF59AC

  attr_reader :file_name
  attr_reader :data

  def read!(arc_file, file_id, header)
    @file_name = read_file_name!(arc_file, file_id)

    data_origin,
    data_size = arc_file.read(8).unpack('LL')
    data_size ^= ENCRYPTION_KEY

    @data = lambda do |arc_file|
      arc_file.seek(data_origin, IO::SEEK_SET)
      data = arc_file.read(data_size)
      data = decrypt(data) if header.encrypted
      data
    end
  end

  private

  def decrypt(data)
    data_bytes = data.unpack('C*')
    fn_bytes = @file_name.unpack('C*')
    (0..[0x2172, data_bytes.length - 1].min).each do |i|
      data_bytes[i] ^= (fn_bytes[i % fn_bytes.length] + i + 3) & 0xff
    end

    data_bytes.pack('C*')
  end

  def read_file_name!(arc_file, file_id)
    file_name = arc_file.read(60).unpack('C*')
    (0..58).each { |i| file_name[i] ^= (file_id * i * 3 + 0x3d) & 0xff }
    file_name = file_name[0..(file_name.index(0) - 1)]
    file_name.pack('C*')
  end
end
