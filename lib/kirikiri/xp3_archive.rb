require 'zlib'
require 'stringio'
require_relative '../archive'
require_relative 'xp3_header'
require_relative 'xp3_file_table'
require_relative 'xp3_file_entry'
require_relative 'xp3_info_chunk'
require_relative 'xp3_segm_chunk'
require_relative 'xp3_adlr_chunk'

# XP3 archive
class Xp3Archive < Archive
  def initialize(decryptor)
    @decryptor = decryptor
  end

  def read(path)
    super
    open(path, 'rb') do |file|
      @header = Xp3Header.new.read!(file)
      file.seek(@header.file_table_origin, IO::SEEK_SET)
      @file_table = Xp3FileTable.new.read!(file)
    end
  end

  def extract_file(file_entry, input_file, target_path)
    file_entry.extract(
      input_file,
      target_path,
      ->(data, file_entry) { @decryptor.filter(data, file_entry) })
  end
end
