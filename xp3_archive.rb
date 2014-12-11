require 'zlib'
require 'stringio'
require 'awesome_print'

# XP3 archive
class Xp3Archive
  def initialize(path, decryptor)
    @path = path
    @decryptor = decryptor
  end

  def read!
    open(@path, 'rb') do |file|
      @header = Xp3Header.new.read!(file)
      file.seek(@header.file_table_origin, IO::SEEK_SET)
      @file_table = Xp3FileTable.new.read!(file)
    end
  end

  def extract(output_dir, verbose)
    open(@path, 'rb') do |input_file|
      @file_table.files.each do |xp3_file|
        target_path = File.join(output_dir, xp3_file.info_chunk.file_name)
        FileUtils.mkpath(File.dirname(target_path))
        print 'Extracting to ' + target_path + '... ' if verbose
        begin
          xp3_file.extract(
            input_file,
            target_path,
            ->(data, file_entry) { @decryptor.filter(data, file_entry) })
        rescue StandardError => e
          puts e.message if verbose
        else
          puts 'ok' if verbose
        end
      end
    end
  end

  # basic XP3 header (not fully reversed)
  class Xp3Header
    attr_accessor :magic
    attr_accessor :version
    attr_accessor :file_table_origin

    def read!(file)
      @magic = file.read(5)
      @version = file.read(4).unpack('I')[0]
      file.seek(2, IO::SEEK_CUR)
      @file_table_origin = file.read(4).unpack('I')[0]
      self
    end
  end

  # XP3 file table
  class Xp3FileTable
    attr_accessor :files

    def read!(file)
      raw = StringIO.new(read_raw_file_table!(file))
      @files = []
      @files.push(Xp3FileEntry.new.read!(raw)) until raw.eof?
      self
    end

    def read_raw_file_table!(file)
      use_zlib = file.read(1).unpack('B')[0]
      if use_zlib
        file_table_compressed_size = file.read(8).unpack('Q<')[0]
        file.seek(8, IO::SEEK_CUR) # 64 bit uncompressed file table size
        return Zlib.inflate(file.read(file_table_compressed_size))
      else
        file_table_size = file.read(8).unpack('Q')[0]
        return file.read(file_table_size)
      end
    end
  end

  # XP3 file table entry
  class Xp3FileEntry
    attr_accessor :info_chunk
    attr_accessor :segm_chunks
    attr_accessor :adlr_chunk

    def read!(file)
      magic = file.read(4)
      fail 'Expected file chunk' unless magic == 'File'
      file_chunk_size = file.read(8).unpack('Q<')[0]
      file_chunk = StringIO.new(file.read(file_chunk_size))

      @info_chunk = Xp3InfoChunk.new.read!(file_chunk)
      @segm_chunks = Xp3SegmChunk.read_list!(file_chunk)
      @adlr_chunk = Xp3AdlrChunk.new.read!(file_chunk)
      self
    end

    def extract(handle, target_path, filter)
      open(target_path, 'wb') do |output_file|
        data = ''
        @segm_chunks.each { |segm_chunk| data += segm_chunk.read_data!(handle) }
        data = filter.call(data, self)
        output_file.write(data)
      end
    end
  end

  # XP3 INFO chunk
  class Xp3InfoChunk
    attr_accessor :protect
    attr_accessor :original_file_size
    attr_accessor :compressed_file_size
    attr_accessor :file_name

    def read!(file)
      magic = file.read(4)
      fail 'Expected info chunk' unless magic == 'info'
      info_chunk_size = file.read(8).unpack('Q<')[0]
      info_chunk = StringIO.new(file.read(info_chunk_size))

      @protect = info_chunk.read(4).unpack('I<')[0]
      @original_file_size = info_chunk.read(8).unpack('Q<')[0]
      @compressed_file_size = info_chunk.read(8).unpack('Q<')[0]

      name_length = info_chunk.read(2).unpack('S<')[0]
      @file_name =
        info_chunk
        .read(name_length * 2)
        .force_encoding('utf-16le')
        .encode('utf-8')
      self
    end
  end

  # Xp3 SEGM chunk
  class Xp3SegmChunk
    attr_accessor :flags
    attr_accessor :offset
    attr_accessor :original_size
    attr_accessor :compressed_size

    def self.read_list!(file)
      magic = file.read(4)
      fail 'Expected segment chunk' unless magic == 'segm'
      segm_chunks_size = file.read(8).unpack('Q<')[0]
      segm_chunks = StringIO.new(file.read(segm_chunks_size))

      @chunks = []
      @chunks.push(Xp3SegmChunk.new.read!(segm_chunks)) until segm_chunks.eof?
      @chunks
    end

    def read_data!(file)
      file.seek(@offset, IO::SEEK_SET)
      use_zlib = @flags & 7 == 1
      if use_zlib
        return Zlib.inflate(file.read(@compressed_size))
      else
        return file.read(@original_size)
      end
    end

    def read!(file)
      @flags = file.read(4).unpack('L<')[0]
      @offset = file.read(8).unpack('Q<')[0]
      @original_size = file.read(8).unpack('Q<')[0]
      @compressed_size = file.read(8).unpack('Q<')[0]
      self
    end
  end

  # XP3 ADLR chunk
  class Xp3AdlrChunk
    attr_accessor :encryption_key

    def read!(file)
      magic = file.read(4)
      fail 'Expected ADLR chunk' unless magic == 'adlr'
      adlr_chunk_size = file.read(8).unpack('Q<')[0]
      adlr_chunk = StringIO.new(file.read(adlr_chunk_size))

      @encryption_key = adlr_chunk.read(4).unpack('L<')
      self
    end
  end
end
