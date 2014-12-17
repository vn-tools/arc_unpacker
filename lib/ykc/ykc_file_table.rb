require_relative 'ykc_file_entry'

# YKC file table
class YkcFileTable
  attr_reader :files

  def read!(file, ykc_header)
    file.seek(ykc_header.file_table_origin)
    num_files = ykc_header.file_table_size / 20

    @files = []
    (0..(num_files - 1)).each do
      @files.push(YkcFileEntry.new.read!(file))
    end
    self
  end
end
