# Generic file
class FileEntry
  attr_reader :file_name

  # Since storing full data of each file in the memory is unwise,
  # this is used as a lambda that fetches the file content in a lazy fashion.
  attr_reader :data

  def initialize(file_name, data)
    @file_name, @data = file_name, data
  end
end
