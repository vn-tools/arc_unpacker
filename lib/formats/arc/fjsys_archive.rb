require 'lib/formats/gfx/mgd_converter'
require 'lib/formats/script/msd_converter'
require 'lib/virtual_file'

# FJSYS archive
# Company: various
# Extension: none
# Known games:
# - Sono Hanabira ni Kuchizuke o 1
# - Sono Hanabira ni Kuchizuke o 2
# - Sono Hanabira ni Kuchizuke o 3
# - Sono Hanabira ni Kuchizuke o 4
# - Sono Hanabira ni Kuchizuke o 5
# - Sono Hanabira ni Kuchizuke o 6
# - Sono Hanabira ni Kuchizuke o 7
# - Sono Hanabira ni Kuchizuke o 8
# - Sono Hanabira ni Kuchizuke o 9
# - Sono Hanabira ni Kuchizuke o 10
# - Sono Hanabira ni Kuchizuke o 11
module FjsysArchive
  module_function

  MAGIC = "FJSYS\x00\x00\x00"

  def add_cli_help(arg_parser)
    MsdConverter.add_cli_help(arg_parser)
    MgdConverter.add_cli_help(arg_parser)
  end

  def parse_cli_options(arg_parser, options)
    MsdConverter.parse_cli_options(arg_parser, options)
    MgdConverter.parse_cli_options(arg_parser, options)
  end

  class Unpacker
    def unpack(arc_file, output_files, options)
      magic = arc_file.read(MAGIC.length)
      fail RecognitionError, 'Not a FJSYS archive' unless magic == MAGIC

      header_size,
      file_names_size,
      file_count = arc_file.read(76).unpack('LLLx64')
      file_names_start = header_size - file_names_size

      file_count.times do
        output_files.write do
          file_name_origin,
          data_size,
          data_origin = arc_file.read(16).unpack('LLQ')

          file_name = arc_file.peek(file_name_origin + file_names_start) do
            arc_file.read_until_zero
          end

          data = arc_file.peek(data_origin) { arc_file.read(data_size) }

          file = VirtualFile.new(file_name, data)
          decode!(file, options)
          file
        end
      end
    end

    private

    def decode!(file, options)
      if file.data.start_with?(MgdConverter::MAGIC)
        MgdConverter.decode!(file, options)
      elsif file.name.downcase.end_with?('.msd')
        MsdConverter.decode!(file, options)
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, _options)
      arc_file.write(MAGIC)

      file_names_start = input_files.length * 16 + 0x54
      file_names_size = input_files.names.map { |f| f.length + 1 }.reduce(0, :+)

      header_size = file_names_size + file_names_start

      arc_file.write([
        header_size,
        file_names_size,
        input_files.length].pack('LLLx64'))
      arc_file.write("\x00" * (header_size - arc_file.tell))

      table_entries = []
      input_files.each do |file|
        table_entries.push(
          file_name: file.name,
          data_size: file.data.length,
          data_origin: arc_file.tell)

        arc_file.write(file.data)
      end

      table_entries = fix_file_order(table_entries)

      arc_file.seek(file_names_start)
      table_entries.each do |e|
        e[:file_name_origin] = arc_file.tell - file_names_start
        arc_file.write(e[:file_name])
        arc_file.write("\x00")
      end

      arc_file.seek(0x54)
      table_entries.each do |e|
        arc_file.write([
          e[:file_name_origin],
          e[:data_size],
          e[:data_origin]].pack('LLQ'))
      end
    end

    private

    # it is important to sort the files like the game did,
    # because the game refers to the file by their indices, not the file names!
    def fix_file_order(table_entries)
      # I guess the files are sorted alphabetically, but this assumption might
      # be wrong. I know that what I do below is stupid, but "_" needed to be
      # placed before "0" and after "." in the games I tested.
      table_entries.sort_by { |e| e[:file_name].gsub('_', '/').downcase }
    end
  end
end
