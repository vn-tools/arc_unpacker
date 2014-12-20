require 'fileutils'

# Generic archive
class Archive
  attr_reader :files

  def initialize
    @files = []
  end

  def read(path) # rubocop:disable Style/TrivialAccessors
    @path = path
    open(path, 'rb') do |arc_file|
      read_internal(arc_file)
    end
  end

  def extract(output_dir, verbose)
    open(@path, 'rb') do |arc_file|
      @files.each do |file_entry|
        target_path = File.join(output_dir, file_entry.file_name)
        FileUtils.mkpath(File.dirname(target_path))

        print 'Extracting to ' + target_path + '... ' if verbose
        begin
          data = file_entry.data.call(arc_file)
          open(target_path, 'wb') do |output_file|
            output_file.write(data)
          end
        rescue StandardError => e
          puts e.message if verbose
        else
          puts 'ok' if verbose
        end
      end
    end
  end
end
