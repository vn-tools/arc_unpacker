require 'fileutils'
require 'json'

# A class used to save extracted archive resources to disk
class OutputFiles
  # A file that is used to contain data necessary to repack some files.
  # For example, graphic files that need tags.
  META_FILE_NAME = 'arc_meta.txt'

  def initialize(target_dir, options)
    @target_dir = target_dir
    @verbosity = options[:verbosity]
  end

  def write(&block)
    print 'Extracting... ' if @verbosity != :quiet

    file_name, data = block.call
    target_path = File.join(@target_dir, file_name.gsub('\\', '/'))

    FileUtils.mkpath(File.dirname(target_path))
    File.binwrite(target_path, data)
  rescue StandardError => e
    puts e.message if @verbosity != :quiet
    puts e.backtrace if @verbosity == :debug
  else
    puts 'ok (saved in ' + target_path + ')' if @verbosity != :quiet
  end

  def write_meta(meta)
    target_path = File.join(@target_dir, META_FILE_NAME)
    File.binwrite(target_path, JSON.dump(meta))
  end
end
