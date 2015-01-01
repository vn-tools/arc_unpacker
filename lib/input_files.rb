require 'json'
require 'pathname'
require_relative 'output_files'

# A class used to supply packer with input files read from disk
class InputFiles
  attr_reader :names

  def initialize(source_dir, options)
    @source_dir = source_dir
    @verbosity = options[:verbosity]
    @paths = []
    @names = []

    Dir[source_dir + '/**/*'].each do |path|
      next unless File.file?(path)
      next if path.end_with?(OutputFiles::META_FILE_NAME)

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
    source_path = File.join(@source_dir, OutputFiles::META_FILE_NAME)
    return nil unless File.exist?(source_path)
    JSON.parse(File.binread(source_path), symbolize_names: true)
  end

  private

  def pack(file_path, file_name, &block)
    print format('Inserting %s... ', file_name) if @verbosity != :quiet
    file_data = File.binread(file_path)
    block.call(file_name, file_data)
  rescue StandardError => e
    puts e.message if @verbosity != :quiet
    puts e.backtrace if @verbosity == :debug
  else
    puts 'ok' if @verbosity != :quiet
  end
end
