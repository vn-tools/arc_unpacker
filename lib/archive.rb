require 'fileutils'
require 'pathname'
require 'json'

# Generic archive
class Archive
  attr_reader :files
  attr_reader :meta

  def initialize
    @files = []
    @meta = nil
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

  def add_files(input_dir)
    Dir[input_dir + '/**/*'].each do |path|
      next unless File.file?(path)

      if path.end_with?(meta_file_name)
        @meta = JSON.parse(File.binread(path), symbolize_names: true)
        next
      end

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
      extract_file(output_dir, file_entry, verbosity)
    end

    return if @meta.nil?
    File.binwrite(File.join(output_dir, meta_file_name), JSON.dump(@meta))
  end

  protected

  def read_internal(_arc_file)
    fail 'This format does not support unpacking'
  end

  def write_internal(_arc_file, _options)
    fail 'This format does not support packing'
  end

  private

  def extract_file(output_dir, file_entry, verbosity)
    target_path = File.join(output_dir, file_entry.file_name)
    print 'Extracting to ' + target_path + '... ' if verbosity != :quiet

    FileUtils.mkpath(File.dirname(target_path))

    data = file_entry.data.call
    open(target_path, 'wb') { |output_file| output_file.write(data) }
  rescue StandardError => e
    puts e.message if verbosity != :quiet
    puts e.backtrace if verbosity == :debug
  else
    puts 'ok' if verbosity != :quiet
  end

  # A file that is used to contain data necessary to repack some files.
  # For example, graphic files that need tags.
  def meta_file_name
    'arc_meta.txt'
  end
end
