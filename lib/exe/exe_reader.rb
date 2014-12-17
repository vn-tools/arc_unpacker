require_relative '../archive'
require_relative 'exe_resources_reader'

# Windows executable reader
class ExeReader < Archive
  def read(path)
    super
    open(path, 'rb') do |file|
      @file_table = ExeResourcesReader.new(file)
    end
  end
end
