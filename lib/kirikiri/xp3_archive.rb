require 'zlib'
require 'stringio'
require_relative 'xp3_header'
require_relative 'xp3_file_table'
require_relative 'xp3_file_entry'
require_relative 'xp3_info_chunk'
require_relative 'xp3_segm_chunk'
require_relative 'xp3_adlr_chunk'

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
end
