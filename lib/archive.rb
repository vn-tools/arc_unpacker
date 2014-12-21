require 'fileutils'
require 'pathname'

# Generic archive
class Archive
  attr_reader :files

  def initialize
    @files = []
  end

  def read(path)
    arc_file = File.open(path, 'rb')
    read_internal(arc_file)
    # allow FileEntry.data lambda func to use the file descriptor
    # by destroying it at a later time.
    ObjectSpace.define_finalizer(self, proc { arc_file.close })
  end

  def write(path, options = {})
    open(path, 'wb') { |arc_file| write_internal(arc_file, options) }
  end

  def read_internal(_arc_file)
    fail 'This format does not support unpacking'
  end

  def write_internal(_arc_file, _options)
    fail 'This format does not support packing'
  end

  def add_files(input_dir)
    Dir[input_dir + '/**/*'].each do |path|
      next unless File.file?(path)

      file_name =
        Pathname.new(path)
        .relative_path_from(Pathname.new(input_dir))
        .to_s

      data = -> { open(path, 'rb') { |h| h.read } }

      @files.push(FileEntry.new(file_name, data))
    end
  end

  def extract(output_dir, verbosity)
    @files.each do |file_entry|
      target_path = File.join(output_dir, file_entry.file_name)
      FileUtils.mkpath(File.dirname(target_path))

      print 'Extracting to ' + target_path + '... ' if verbosity != :quiet
      begin
        data = file_entry.data.call
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
