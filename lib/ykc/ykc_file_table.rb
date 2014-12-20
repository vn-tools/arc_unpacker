require_relative 'ykc_file_entry'

# YKC file table
class YkcFileTable
  attr_reader :files

  def initialize
    @files = []
  end

  def read!(arc_file, ykc_header)
    arc_file.seek(ykc_header.file_table_origin)
    num_files = ykc_header.file_table_size / 20

    @files = (1..num_files).map do
      entry = YkcFileEntry.new
      entry.read!(arc_file)
      entry
    end
  end
end
