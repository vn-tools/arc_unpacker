require_relative '../archive'
require_relative '../file_entry'

# Melty Blood archive
class MeltyBloodArchive < Archive
  MAGIC1 = "\x00\x00\x00\x00" # encrypted
  MAGIC2 = "\x01\x00\x00\x00" # not encrypted
  ENCRYPTION_KEY = 0xE3DF59AC

  def read_internal(arc_file)
    magic = arc_file.read(4)
    encrypted = magic == MAGIC1
    fail 'Not a Melty Blood archive' unless magic == MAGIC1 || magic == MAGIC2

    num_files = arc_file.read(4).unpack('L<')[0] ^ ENCRYPTION_KEY
    @files = (1..num_files).map do |i|
      read_file(arc_file, i - 1, encrypted)
    end
  end

  private

  def read_file(arc_file, file_id, encrypted)
    file_name = read_file_name(arc_file, file_id)

    data_origin,
    data_size = arc_file.read(8).unpack('LL')
    data_size ^= ENCRYPTION_KEY

    data = lambda do |arc_file|
      arc_file.seek(data_origin, IO::SEEK_SET)
      data = arc_file.read(data_size)
      data = decrypt(data, file_name) if encrypted
      data
    end

    FileEntry.new(file_name, data)
  end

  def decrypt(data, file_name)
    data_bytes = data.unpack('C*')
    fn_bytes = file_name.unpack('C*')
    (0..[0x2172, data_bytes.length - 1].min).each do |i|
      data_bytes[i] ^= (fn_bytes[i % fn_bytes.length] + i + 3) & 0xff
    end

    data_bytes.pack('C*')
  end

  def read_file_name(arc_file, file_id)
    file_name = arc_file.read(60).unpack('C*')
    (0..58).each { |i| file_name[i] ^= (file_id * i * 3 + 0x3d) & 0xff }
    file_name = file_name[0..(file_name.index(0) - 1)]
    file_name.pack('C*')
  end
end
