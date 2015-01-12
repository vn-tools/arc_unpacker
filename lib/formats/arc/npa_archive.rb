require 'lib/binary_io'
require 'lib/virtual_file'
require 'zlib'
require_relative 'npa_archive/filter_factory'

# NPA archive
# Company: Nitroplus
# Extension: .npa
# Known games:
# - Chaos;Head
module NpaArchive
  module_function

  MAGIC = "NPA\x01\x00\x00\x00".b
  FILE_TYPE_DIRECTORY = 1
  FILE_TYPE_FILE = 2

  def add_cli_help(arg_parser)
    arg_parser.add_help(
      '--plugin=PLUGIN',
      'Selects NPA decryption routine.',
      possible_values: NPA_FILTERS.keys)
  end

  def parse_cli_options(arg_parser, options)
    filter = arg_parser.switch(['--plugin'])
    options[:filter] = filter
  end

  def get_filter(symbol)
    filter = NPA_FILTERS[symbol.to_sym]
    fail RecognitionError, 'Unknown filter' if filter.nil?
    filter.call
  end

  class Unpacker
    def unpack(arc_file, output_files, options)
      @filter = NpaArchive.get_filter(options[:filter])

      magic = arc_file.read(MAGIC.length)
      fail RecognitionError, 'Not a NPA archive' unless magic == MAGIC

      header = read_header(arc_file)
      table = read_table(arc_file, header)
      read_contents(arc_file, header, table, output_files)
    end

    private

    def read_header(arc_file)
      header = {}
      header[:key1],
      header[:key2],
      header[:compressed],
      header[:encrypted],
      header[:total_count],
      header[:folder_count],
      header[:file_count],
      header[:table_size] = arc_file.read(34).unpack('L2 C2 L3 x8 L')

      %w(compressed encrypted).map(&:to_sym).each do |x|
        header[x] = header[x] > 0
      end

      header
    end

    def read_table(arc_file, header)
      table_origin = arc_file.tell

      table = []
      header[:total_count].times do |file_id|
        name_length = arc_file.read(4).unpack('L')[0]
        name = arc_file.read(name_length)
        name = decrypt_file_name(name, file_id, header)

        e = { name: name, real_file_id: file_id }
        e[:type],
        e[:file_id],
        e[:origin],
        e[:size_compressed],
        e[:size_original] = arc_file.read(17).unpack('C L4')
        e[:origin] += table_origin + header[:table_size]

        case e[:type]
        when FILE_TYPE_DIRECTORY
          next
        when FILE_TYPE_FILE
          table.push(e)
        else
          fail RecognitionError, 'Unknown file type: ' + e[:type].to_s
        end
      end

      if table_origin + header[:table_size] != arc_file.tell
        fail RecognitionError, 'Bad file table size.'
      end

      table
    end

    def read_contents(arc_file, header, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) do
            arc_file.read(e[:size_compressed])
          end

          data = decrypt_data(data, e, header) if header[:encrypted]
          data = Zlib.inflate(data) if e[:size_compressed] != e[:size_original]

          unless data.length == e[:size_original]
            fail RecognitionError, 'Bad file size'
          end

          VirtualFile.new(e[:name], data)
        end
      end
    end

    def name_bytes(name)
      name.encode('sjis').unpack('C*')
    end

    def file_name_key(key1, key2, file_id, i)
      tmp = @filter.file_name_key(key1, key2)
      key = 0xfc * i
      key -= tmp >> 0x18
      key -= tmp >> 0x10
      key -= tmp >> 0x08
      key -= tmp & 0xff
      key -= file_id >> 0x18
      key -= file_id >> 0x10
      key -= file_id >> 0x08
      key -= file_id
      key & 0xff
    end

    def data_key(table_entry, header)
      name_bytes = name_bytes(table_entry[:name])
      key = @filter.data_key
      key -= name_bytes.reduce(0, &:+)
      key *= name_bytes.length
      key += header[:key1] * header[:key2]
      key *= table_entry[:size_original]
      key & 0xff
    end

    def decrypt_data(data, table_entry, header)
      perm = @filter.permutation
      key = data_key(table_entry, header)
      len = 0x1000 + name_bytes(table_entry[:name]).length
      len = [len, data.length].min
      len.times { |i| data[i] = ((perm[data[i].ord] - key - i) & 0xff).chr }
      data
    end

    def decrypt_file_name(name, file_id, header)
      decrypted = ''
      name.length.times do |i|
        key = file_name_key(header[:key1], header[:key2], file_id, i)
        decrypted += (((name[i].ord + key) & 0xff).chr)
      end
      decrypted.force_encoding('sjis').encode('utf-8')
    end
  end
end
