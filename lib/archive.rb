require 'fileutils'
require 'pathname'

# Generic archive
class Archive
  attr_reader :files

  def initialize
    @files = []
  end

  def read(path) # rubocop:disable Style/TrivialAccessors
    @path = path
    open(path, 'rb') { |arc_file| read_internal(arc_file) }
  end

  def write(path)
    @path = path
    open(path, 'wb') { |arc_file| write_internal(arc_file) }
  end

  def read_internal(_arc_file)
    fail 'This format does not support unpacking'
  end

  def write_internal(_arc_file)
    fail 'This format does not support packing'
  end

  def add_files(input_dir)
    Dir[input_dir + '/**/*'].each do |path|
      next unless File.file?(path)

      file_name =
        Pathname.new(path)
        .relative_path_from(Pathname.new(input_dir))
        .to_s

      data = ->(_arc_file) { open(path, 'rb') { |h| h.read } }

      @files.push(FileEntry.new(file_name, data))
    end
  end

  def extract(output_dir, verbosity)
    open(@path, 'rb') do |arc_file|
      @files.each do |file_entry|
        target_path = File.join(output_dir, file_entry.file_name)
        FileUtils.mkpath(File.dirname(target_path))

        print 'Extracting to ' + target_path + '... ' if verbosity != :quiet
        begin
          data = file_entry.data.call(arc_file)
          open(target_path, 'wb') { |output_file| output_file.write(data) }
        rescue StandardError => e
          puts e.message if verbosity != :quiet
          puts e.backtrace if verbosity == :debug
        else
          puts 'ok' if verbosity != :quiet
        end
      end
    end
  end
end
