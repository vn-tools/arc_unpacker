require 'lib/formats/gfx/spb_converter'
require 'lib/virtual_file'
require_relative 'nsa_archive/lzss_compressor'

# NSA archive
# Engine: Nscripter
# Extension: .nsa
# Known games:
# - Tsukihime
module NsaArchive
  module_function

  NO_COMPRESSION = 0
  SPB_COMPRESSION = 1
  LZSS_COMPRESSION = 2

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def lzss_compressor
    LzssCompressor.new(initial_dictionary_pos: 239, reuse_compressed: true)
  end

  class Unpacker
    def unpack(arc_file, output_files, _options)
      table = read_table(arc_file)
      read_contents(arc_file, table, output_files)
    end

    private

    def read_table(arc_file)
      num_files,
      offset_to_files = arc_file.read(6).unpack('S>L>')
      if offset_to_files > arc_file.size
        fail RecognitionError, 'Bad offset to files'
      end

      table = []
      num_files.times do
        e = {}

        e[:name] = arc_file.read_until_zero
        e[:compression_type],
        e[:origin],
        e[:size_compressed],
        e[:size_original] = arc_file.read(13).unpack('CL>L>L>')

        e[:origin] += offset_to_files
        table.push(e)

        if e[:origin] + e[:size_compressed] > arc_file.size
          fail RecognitionError, 'Bad offset to file'
        end
      end
      table
    end

    def read_contents(arc_file, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) do
            arc_file.read(e[:size_compressed])
          end

          file = VirtualFile.new(e[:name], data)
          decode!(file, e)

          unless file.data.length == e[:size_original]
            fail RecognitionError, 'Bad file size'
          end

          file
        end
      end
    end

    def decode!(file, table_entry)
      case table_entry[:compression_type]
      when SPB_COMPRESSION
        SpbConverter.decode!(file, {})
        table_entry[:size_original] = file.data.length
      when LZSS_COMPRESSION
        file.data = NsaArchive.lzss_compressor.decode(file.data)
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, options)
      table_size = input_files.names.map { |n| n.length + 14 }.reduce(0, :+)
      offset_to_files = 6 + table_size
      arc_file.write([input_files.length, offset_to_files].pack('S>L>'))
      arc_file.write("\x00" * table_size)

      compression_type = options[:compression] || NO_COMPRESSION
      cur_data_origin = 0
      table_entries = []
      input_files.each do |file|
        data_size_original = file.data.length
        encode!(file, compression_type)
        data_size_compressed = file.data.length

        arc_file.write(file.data)

        table_entries.push([
          file.name,
          cur_data_origin,
          data_size_original,
          data_size_compressed])

        cur_data_origin += data_size_compressed
      end

      arc_file.seek(6)
      table_entries.each do |file_name, data_origin, orig_size, compressed_size|
        arc_file.write(file_name.gsub('/', '\\'))
        arc_file.write("\0")

        arc_file.write([
          compression_type,
          data_origin,
          compressed_size,
          orig_size].pack('CL>L>L>'))
      end
    end

    private

    def encode!(file, compression_type)
      case compression_type
      when SPB_COMPRESSION
        SpbConverter.encode!(file, {})
      when LZSS_COMPRESSION
        file.data = NsaArchive.lzss_compressor.encode(file.data)
      end
    end
  end
end
