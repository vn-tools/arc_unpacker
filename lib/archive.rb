require 'fileutils'
require 'pathname'
require 'json'
require_relative 'binary_io'

# Generic archive
class Archive
  # A file that is used to contain data necessary to repack some files.
  # For example, graphic files that need tags.
  META_FILE_NAME = 'arc_meta.txt'

  def unpack(source_arc, target_dir, verbosity)
    BinaryIO.from_file(source_arc, 'rb') do |arc_file|
      unpack_internal(arc_file, OutputFiles.new(target_dir, verbosity))
    end
  end

  def pack(source_dir, target_arc)
    BinaryIO.from_file(target_arc, 'wb') do |arc_file|
      pack_internal(arc_file, InputFiles.new(source_dir), {})
    end
  end

  protected

  def unpack_internal(_arc_file, _output_files)
    fail 'This format does not support unpacking'
  end

  def pack_internal(_arc_file, _input_files, _options)
    fail 'This format does not support packing'
  end

  # A class used to save extracted archive resources to disk
  class OutputFiles
    def initialize(target_dir, verbosity)
      @target_dir = target_dir
      @verbosity = verbosity
    end

    def write(file_name, data)
      target_path = File.join(@target_dir, file_name)
      print 'Extracting to ' + target_path + '... ' if @verbosity != :quiet

      FileUtils.mkpath(File.dirname(target_path))
      File.binwrite(target_path, data)
    rescue StandardError => e
      puts e.message if @verbosity != :quiet
      puts e.backtrace if @verbosity == :debug
    else
      puts 'ok' if @verbosity != :quiet
    end

    def write_meta(meta)
      target_path = File.join(@target_dir, META_FILE_NAME)
      File.binwrite(target_path, JSON.dump(meta))
    end
  end

  # A class used to supply packer with input files read from disk
  class InputFiles
    attr_reader :names

    def initialize(source_dir)
      @source_dir = source_dir
      @paths = []
      @names = []

      Dir[source_dir + '/**/*'].each do |path|
        next unless File.file?(path)
        next if path.end_with?(META_FILE_NAME)

        file_name =
          Pathname.new(path)
          .relative_path_from(Pathname.new(source_dir))
          .to_s

        @paths.push(path)
        @names.push(file_name)
      end
    end

    def each(&block)
      @paths.zip(@names).each { |fp, fn| pack(fp, fn, &block) }
    end

    def reverse_each(&block)
      @paths.zip(@names).reverse_each { |fp, fn| pack(fp, fn, &block) }
    end

    def length
      @paths.length
    end

    def read_meta
      source_path = File.join(@source_dir, META_FILE_NAME)
      JSON.parse(File.binread(source_path), symbolize_names: true)
    end

    private

    def pack(file_path, file_name, &block)
      file_data = File.binread(file_path)
      block.call(file_name, file_data)
    end
  end
end
