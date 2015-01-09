# Melty Blood archive
# Company: French Bread
# Extension: .p
# Known games:
# - Melty Blood
module MeltyBloodArchive
  module_function

  MAGIC1 = "\x00\x00\x00\x00" # encrypted
  MAGIC2 = "\x01\x00\x00\x00" # not encrypted
  ENCRYPTION_KEY = 0xE3DF59AC

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  class Unpacker
    def unpack(arc_file, output_files, _options)
      magic = arc_file.read(4)
      encrypted = magic == MAGIC1
      unless [MAGIC1, MAGIC2].include? magic
        fail ArcError, 'Not a Melty Blood archive'
      end

      table = read_table(arc_file)
      read_contents(arc_file, table, output_files, encrypted)
    end

    private

    def read_table(arc_file)
      num_files = arc_file.read(4).unpack('L<')[0] ^ ENCRYPTION_KEY
      table = []
      num_files.times do |i|
        e = { name: read_file_name(arc_file, i), file_id: i }
        e[:origin],
        e[:size] = arc_file.read(8).unpack('LL')
        e[:size] ^= ENCRYPTION_KEY
        table << e

        if e[:origin] + e[:size] > arc_file.size
          fail ArcError, 'Bad offset to file'
        end
      end
      table
    end

    def read_contents(arc_file, table, output_files, encrypted)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) do
            data = arc_file.read(e[:size])
            data = decrypt(data, e[:name]) if encrypted
            data
          end

          [e[:name], data]
        end
      end
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
end
