# Generic file
class FileEntry
  attr_reader :file_name
  attr_reader :data # lambda

  def initialize(file_name, data)
    @file_name, @data = file_name, data
  end
end
