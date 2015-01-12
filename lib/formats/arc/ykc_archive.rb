require 'lib/formats/gfx/ykg_converter'
require 'lib/formats/script/yks_converter'
require 'lib/memory_file'

# YKC archive
# Engine: YukaScript
# Extension: .ykc
# Known games:
# - Hoshizora e Kakaru Hashi
module YkcArchive
  module_function

  MAGIC = 'YKC001'

  def add_cli_help(arg_parser)
    YkgConverter.add_cli_help(arg_parser)
    YksConverter.add_cli_help(arg_parser)
  end

  def parse_cli_options(arg_parser, options)
    YkgConverter.parse_cli_options(arg_parser, options)
    YksConverter.parse_cli_options(arg_parser, options)
  end

  class Unpacker
    def unpack(arc_file, output_files, options)
      magic = arc_file.read(MAGIC.length)
      fail RecognitionError, 'Not a YKC archive' unless magic == MAGIC

      _version,
      table_origin,
      table_size = arc_file.read(18).unpack('xxLx4L2')

      arc_file.seek(table_origin)
      num_files = table_size / 20

      num_files.times do
        output_files.write do
          name_origin,
          name_size,
          data_origin,
          data_size = arc_file.read(20).unpack('L4 x4')

          name = arc_file.peek(name_origin) { arc_file.read(name_size - 1) }
          data = arc_file.peek(data_origin) { arc_file.read(data_size) }

          file = MemoryFile.new(name, data)
          decode!(file, options)
          file
        end
      end
    end

    private

    def decode!(file, options)
      if file.data.start_with?(YkgConverter::MAGIC)
        file.data = YkgConverter.decode(file.data, options)
      elsif file.data.start_with?(YksConverter::MAGIC)
        file.data = YksConverter.decode(file.data, options)
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, options)
      table_origin = MAGIC.length + 18
      table_size = input_files.length * 20

      version = 24
      arc_file.write(MAGIC)
      arc_file.write([version, table_origin, table_size].pack('xxLx4L2'))
      arc_file.write("\x00" * table_size)

      table_entries = {}
      input_files.each do |file|
        encode!(file, options)

        table_entries[file.name] = { name_origin: arc_file.tell }
        arc_file.write(file.name.gsub('/', '\\'))
        arc_file.write("\x00")

        table_entries[file.name][:data_origin] = arc_file.tell
        table_entries[file.name][:data_size] = file.data.length
        arc_file.write(file.data)
      end

      arc_file.seek(table_origin)
      table_entries.each do |name, entry|
        arc_file.write([
          entry[:name_origin],
          name.length + 1,
          entry[:data_origin],
          entry[:data_size]].pack('L4 x4'))
      end
    end

    def encode!(file, options)
      if file.name.downcase.end_with?('.ykg')
        file.data = YkgConverter.encode(file.data, options)
      elsif file.name.downcase.end_with?('.yks')
        file.data = YksConverter.encode(file.data, options)
      end
    end
  end
end
