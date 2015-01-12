require 'lib/binary_io'
require 'lib/memory_file'
require 'zlib'
require_relative 'xp3_archive/filter_factory'

# XP3 archive
# Engine: Kirikiri2
# Extension: .xp3
# Known games:
# - Fate/Stay Night
# - Fate/Hollow Ataraxia
# - Sono Hanabira ni Kuchizuke o 12
module Xp3Archive
  module_function

  MAGIC = "XP3\r\n\x20\x0a\x1a\x8b\x67\x01".b
  FILE_MAGIC = 'File'
  ADLR_MAGIC = 'adlr'
  INFO_MAGIC = 'info'
  SEGM_MAGIC = 'segm'

  def add_cli_help(arg_parser)
    arg_parser.add_help(
      '--plugin=PLUGIN',
      'Selects XP3 decryption routine.',
      possible_values: XP3_FILTERS.keys)
  end

  def parse_cli_options(arg_parser, options)
    filter = arg_parser.switch(['--plugin'])
    filter = filter.nil? ? :none : filter.to_sym
    options[:filter] = filter
  end

  def get_filter(symbol)
    filter = XP3_FILTERS[symbol]
    fail RecognitionError, 'Unknown filter' if filter.nil?
    filter.call
  end

  class Unpacker
    def unpack(arc_file, output_files, options)
      magic = arc_file.read(MAGIC.length)
      fail RecognitionError, 'Not an XP3 archive' unless magic == MAGIC

      version = arc_file.peek(19) { arc_file.read(4) == "\x01\0\0\0" ? 2 : 1 }

      if version == 1
        table_origin = arc_file.read(8).unpack('Q')[0]
      else
        additional_header_offset,
        minor_version = arc_file.read(12).unpack('QI')
        if minor_version != 1
          fail \
            RecognitionError,
            format('Unexpected XP3 version: %s', minor_version)
        end

        arc_file.peek(additional_header_offset) do
          _flag,
          _table_size,
          table_origin = arc_file.read(17).unpack('BQQ')
        end
      end

      table = arc_file.peek(table_origin) { read_raw_table!(arc_file) }
      table = BinaryIO.from_string(table)

      filter = Xp3Archive.get_filter(options[:filter])
      output_files.write { read_file(table, arc_file, filter) } until table.eof?
    end

    private

    def read_raw_table!(arc_file)
      use_zlib = arc_file.read(1).unpack('C')[0] > 0

      if use_zlib
        table_size_compressed,
        table_size_original = arc_file.read(16).unpack('Q<Q<')
        raw = arc_file.read(table_size_compressed)
        raw = Zlib.inflate(raw) if table_size_original != table_size_compressed
        unless raw.length == table_size_original
          fail RecognitionError, 'Bad table size'
        end
        return raw
      end

      raw_size = arc_file.read(8).unpack('Q')[0]
      arc_file.read(raw_size)
    end

    def read_file(raw_table, arc_file, filter)
      magic = raw_table.read(FILE_MAGIC.length)
      fail RecognitionError, 'Expected file chunk' unless magic == FILE_MAGIC

      raw_size = raw_table.read(8).unpack('Q<')[0]
      raw_file_chunk = BinaryIO.from_string(raw_table.read(raw_size))

      info_chunk = Xp3InfoChunk.new
      info_chunk.read!(raw_file_chunk)
      segm_chunks = Xp3SegmChunk.read_list!(raw_file_chunk)
      adlr_chunk = Xp3AdlrChunk.new
      adlr_chunk.read!(raw_file_chunk)

      data = segm_chunks.map { |segm| segm.read_data!(arc_file) } * ''
      data = filter.filter(data, adlr_chunk.encryption_key[0])

      MemoryFile.new(info_chunk.file_name, data)
    end

    # Xp3 SEGM chunk
    class Xp3SegmChunk
      def self.read_list!(arc_file)
        magic = arc_file.read(SEGM_MAGIC.length)
        fail RecognitionError, 'Expected segment chunk' if magic != SEGM_MAGIC

        raw_size = arc_file.read(8).unpack('Q<')[0]
        raw = BinaryIO.from_string(arc_file.read(raw_size))

        chunks = []
        until raw.eof?
          chunk = Xp3SegmChunk.new
          chunk.read!(raw)
          chunks.push(chunk)
        end
        chunks
      end

      def read!(arc_file)
        @flags,
        @offset,
        @original_size,
        @compressed_size = arc_file.read(28).unpack('L<Q<Q<Q<')
      end

      def read_data!(arc_file)
        arc_file.seek(@offset)
        use_zlib = @flags & 7 == 1
        if use_zlib
          raw = Zlib.inflate(arc_file.read(@compressed_size))
          fail RecognitionError, 'Bad SEGM size' if raw.length != @original_size
          return raw
        end

        arc_file.read(@original_size)
      end
    end

    # XP3 INFO chunk
    class Xp3InfoChunk
      attr_accessor :protect
      attr_accessor :original_file_size
      attr_accessor :compressed_file_size
      attr_accessor :file_name

      def read!(arc_file)
        magic = arc_file.read(INFO_MAGIC.length)
        fail RecognitionError, 'Expected info chunk' unless magic == INFO_MAGIC

        raw_size = arc_file.read(8).unpack('Q<')[0]
        raw = BinaryIO.from_string(arc_file.read(raw_size))

        @protect,
        @original_file_size,
        @compressed_file_size,
        name_length = raw.read(22).unpack('I<Q<Q<S<')

        @file_name =
          raw
          .read(name_length * 2)
          .force_encoding('utf-16le')
          .encode('utf-8')
      end
    end

    # XP3 ADLR chunk
    class Xp3AdlrChunk
      attr_accessor :encryption_key

      def read!(arc_file)
        magic = arc_file.read(ADLR_MAGIC.length)
        fail RecognitionError, 'Expected ADLR chunk' unless magic == ADLR_MAGIC

        raw_size = arc_file.read(8).unpack('Q<')[0]
        raw = BinaryIO.from_string(arc_file.read(raw_size))

        @encryption_key = raw.read(4).unpack('L<')
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, options)
      arc_file.write(MAGIC)
      table = prepare_table(input_files, options)
      header_pos = arc_file.tell
      write_dummy_header(arc_file)
      write_contents(arc_file, input_files, table, options)
      table_offset = arc_file.tell
      arc_file.peek(header_pos) { write_header(arc_file, table_offset) }
      write_table(arc_file, table, options)
    end

    private

    def prepare_table(input_files, options)
      table = {}
      origin = 0
      input_files.each do |file|
        e = {
          name: file.name,
          size_original: file.data.length,
          origin: origin + 19,
          random: rand(0xffff_ffff)
        }
        if options[:compress_files]
          e[:size_compressed] = Zlib.deflate(file.data).length
        else
          e[:size_compressed] = file.data.length
        end
        origin += e[:size_compressed]
        table[file.name] = e
      end
      table
    end

    def write_dummy_header(arc_file)
      arc_file.write("\x00" * 8)
    end

    def write_header(arc_file, table_offset)
      arc_file.write([table_offset].pack('Q<'))
    end

    def write_contents(arc_file, input_files, table, options)
      filter = Xp3Archive.get_filter(options[:filter])
      input_files.each do |file|
        file.data = Zlib.deflate(file.data) if options[:compress_files]
        file.data = filter.filter(file.data, table[file.name][:random])
        arc_file.write(file.data)
      end
    end

    def write_table(arc_file, table, options)
      use_zlib = options[:compress_table]

      raw_table = BinaryIO.from_string('')
      table.each do |_name, e|
        info_chunk = prepare_info_chunk(e)
        segm_chunk = prepare_segm_chunk(e, options)
        adlr_chunk = prepare_adlr_chunk(e)
        file_chunk = prepare_file_chunk(info_chunk, segm_chunk, adlr_chunk)
        raw_table.write(file_chunk)
      end

      raw_table.rewind
      raw_table = raw_table.read
      table_size_original = raw_table.length
      raw_table = Zlib.deflate(raw_table) if use_zlib
      table_size_compressed = raw_table.length

      if use_zlib
        arc_file.write("\x01")
        arc_file.write([table_size_compressed].pack('Q<'))
      else
        arc_file.write("\x00")
      end

      arc_file.write([table_size_original].pack('Q<'))
      arc_file.write(raw_table)
    end

    def prepare_file_chunk(info_chunk, segm_chunk, adlr_chunk)
      prepare_chunk(FILE_MAGIC, info_chunk + segm_chunk + adlr_chunk)
    end

    def prepare_info_chunk(table_entry)
      prepare_chunk(INFO_MAGIC, [
        0, # protected?
        table_entry[:size_original],
        table_entry[:size_compressed],
        table_entry[:name].length
      ].pack('I<Q<Q<S<') + table_entry[:name].encode('utf-16le').b)
    end

    def prepare_segm_chunk(table_entry, options)
      prepare_chunk(SEGM_MAGIC, [
        options[:compress_files] ? 1 : 0,
        table_entry[:origin],
        table_entry[:size_original],
        table_entry[:size_compressed]
      ].pack('L<Q<3'))
    end

    def prepare_adlr_chunk(table_entry)
      prepare_chunk(ADLR_MAGIC, [table_entry[:random]].pack('L<'))
    end

    def prepare_chunk(magic, data)
      magic + [data.length].pack('Q<') + data
    end
  end
end
