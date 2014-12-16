require 'fileutils'

# generic archive
class Archive
  def read(path) # rubocop:disable Style/TrivialAccessors
    @path = path
  end

  def extract(output_dir, verbose)
    open(@path, 'rb') do |input_file|
      @file_table.files.each do |file_entry|
        target_path = File.join(output_dir, file_entry.file_name)
        FileUtils.mkpath(File.dirname(target_path))
        print 'Extracting to ' + target_path + '... ' if verbose
        begin
          extract_file(file_entry, input_file, target_path)
        rescue StandardError => e
          puts e.message if verbose
        else
          puts 'ok' if verbose
        end
      end
    end
  end

  def extract_file(file_entry, input_file, target_path)
    file_entry.extract(input_file, target_path)
  end
end
